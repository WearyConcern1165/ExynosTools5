\
/*
 * ExynosTools xeno_wrapper - extended prototype (for CI builds)
 * MIT Licensed. This wrapper improves feature advertising, features2 handling,
 * simple shader cache, performance config loader, autodetect stub, and basic logging.
 *
 * IMPORTANT: This is a user-space wrapper intended for research and testing.
 * It forwards to a vendor ICD via dlopen at runtime. It contains no proprietary code.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/stat.h>
#include <vulkan/vulkan.h>

static void *real_icd = NULL;
static pthread_once_t load_once = PTHREAD_ONCE_INIT;
static char icd_candidates[][256] = {
    "/vendor/lib64/libvkdriver.so",
    "/system/vendor/lib64/libvkdriver.so",
    "/system/lib64/libvulkan.so",
    "/data/data/com.winlator/files/libs/arm64-v8a/libvulkan_radv.so",
    ""
};

static void resolve_symbols(void);

static void load_real_icd_once(void) {
    for (int i = 0; icd_candidates[i][0] != '\\0'; ++i) {
        real_icd = dlopen(icd_candidates[i], RTLD_NOW | RTLD_LOCAL);
        if (real_icd) {
            fprintf(stderr, "[XENO] dlopen success: %s\\n", icd_candidates[i]);
            break;
        }
    }
    if (!real_icd) {
        fprintf(stderr, "[XENO] WARNING: no vendor ICD found, wrapper will be limited.\\n");
    } else {
        resolve_symbols();
    }
}

#define DECL(fn) static PFN_##fn orig_##fn = NULL;
DECL(vkGetInstanceProcAddr)
DECL(vkGetDeviceProcAddr)
DECL(vkCreateInstance)
DECL(vkEnumeratePhysicalDevices)
DECL(vkGetPhysicalDeviceProperties)
DECL(vkGetPhysicalDeviceFeatures2)
DECL(vkCreateDevice)
DECL(vkEnumerateDeviceExtensionProperties)
DECL(vkCreateSwapchainKHR)
DECL(vkQueuePresentKHR)

static void resolve_symbols(void) {
    #define RESOLVE(sym) orig_##sym = (PFN_##sym)dlsym(real_icd, #sym); if (!orig_##sym) { /* non-fatal */ }
    RESOLVE(vkGetInstanceProcAddr)
    RESOLVE(vkGetDeviceProcAddr)
    RESOLVE(vkCreateInstance)
    RESOLVE(vkEnumeratePhysicalDevices)
    RESOLVE(vkGetPhysicalDeviceProperties)
    RESOLVE(vkGetPhysicalDeviceFeatures2)
    RESOLVE(vkCreateDevice)
    RESOLVE(vkEnumerateDeviceExtensionProperties)
    RESOLVE(vkCreateSwapchainKHR)
    RESOLVE(vkQueuePresentKHR)
    #undef RESOLVE
}

static void ensure_loaded(void) {
    pthread_once(&load_once, load_real_icd_once);
}

/* Minimal SHA1-like helper (not crypto-grade) for shader cache keys */
static void simple_hash_hex(const void *data, size_t len, char out_hex[41]) {
    unsigned int h = 1469598103934665603u;
    const unsigned char *p = (const unsigned char*)data;
    for (size_t i = 0; i < len; ++i) h = (h ^ p[i]) * 1099511628211u;
    for (int i = 0; i < 20; ++i) {
        unsigned int v = (h >> (i*3)) & 0xFF;
        sprintf(out_hex + i*2, "%02x", v & 0xFF);
    }
    out_hex[40] = 0;
}

static char shader_cache_dir[512] = "/data/data/com.winlator/files/exynostools_cache";

int shader_cache_get(const void* spirv, size_t spirv_len, void** out_blob, size_t* out_len) {
    if (!spirv || spirv_len == 0) return 0;
    char hex[41]; simple_hash_hex(spirv, spirv_len, hex);
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s.bin", shader_cache_dir, hex);
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    void *buf = malloc(sz);
    if (!buf) { fclose(f); return 0; }
    fread(buf, 1, sz, f); fclose(f);
    *out_blob = buf; *out_len = sz;
    return 1;
}

void shader_cache_put(const void* spirv, size_t spirv_len, const void* blob, size_t blob_len) {
    if (!spirv || spirv_len == 0 || !blob || blob_len == 0) return;
    char hex[41]; simple_hash_hex(spirv, spirv_len, hex);
    char path[1024]; snprintf(path, sizeof(path), "%s/%s.bin", shader_cache_dir, hex);
    FILE *f = fopen(path, "wb"); if (!f) return;
    fwrite(blob, 1, blob_len, f); fclose(f);
}

/* Config loader */
void load_perf_conf(const char* path) {
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\\n') continue;
        char key[256], val[256];
        if (sscanf(line, "%255[^=]=%255s", key, val) == 2) {
            if (strcmp(key, "shader_cache_dir") == 0) {
                strncpy(shader_cache_dir, val, sizeof(shader_cache_dir)-1);
                mkdir(shader_cache_dir, 0755);
            }
        }
    }
    fclose(f);
}

/* Interceptors: advertise extensions and modify features2 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    ensure_loaded();
    if (!orig_vkEnumerateDeviceExtensionProperties) return VK_ERROR_INITIALIZATION_FAILED;
    VkResult res = orig_vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    if (res != VK_SUCCESS && res != VK_INCOMPLETE) return res;
    if (pProperties == NULL) {
        uint32_t origCount = 0;
        orig_vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &origCount, NULL);
        uint32_t extra = 3;
        *pPropertyCount = origCount + extra;
        return VK_SUCCESS;
    }
    uint32_t origCount = 0;
    orig_vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &origCount, NULL);
    VkExtensionProperties *temp = malloc(sizeof(VkExtensionProperties) * origCount);
    if (temp) {
        orig_vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &origCount, temp);
        for (uint32_t i = 0; i < origCount && i < *pPropertyCount; ++i) pProperties[i] = temp[i];
        free(temp);
    }
    uint32_t idx = origCount;
    if (idx < *pPropertyCount) { strncpy(pProperties[idx].extensionName, "VK_EXT_descriptor_indexing", VK_MAX_EXTENSION_NAME_SIZE); pProperties[idx].specVersion = 1; idx++; }
    if (idx < *pPropertyCount) { strncpy(pProperties[idx].extensionName, "VK_EXT_robustness2", VK_MAX_EXTENSION_NAME_SIZE); pProperties[idx].specVersion = 1; idx++; }
    if (idx < *pPropertyCount) { strncpy(pProperties[idx].extensionName, "VK_KHR_shader_float16_int8", VK_MAX_EXTENSION_NAME_SIZE); pProperties[idx].specVersion = 1; idx++; }
    *pPropertyCount = idx;
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    ensure_loaded();
    if (orig_vkGetPhysicalDeviceFeatures2) orig_vkGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    void *p = pFeatures->pNext;
    while (p) {
        VkStructureType s = ((VkBaseOutStructure*)p)->sType;
        if (s == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES) {
            VkPhysicalDeviceDescriptorIndexingFeatures* f = (VkPhysicalDeviceDescriptorIndexingFeatures*)p;
            f->descriptorBindingPartiallyBound = VK_TRUE;
            f->descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        } else if (s == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR) {
            VkPhysicalDeviceFloat16Int8FeaturesKHR* f = (VkPhysicalDeviceFloat16Int8FeaturesKHR*)p;
            f->shaderFloat16 = VK_TRUE;
            f->shaderInt8 = VK_TRUE;
        } else if (s == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT) {
            VkPhysicalDeviceRobustness2FeaturesEXT* f = (VkPhysicalDeviceRobustness2FeaturesEXT*)p;
            f->robustBufferAccess2 = VK_TRUE;
            f->robustImageAccess2 = VK_TRUE;
        }
        p = ((VkBaseOutStructure*)p)->pNext;
    }
}

/* Swapchain intercept for basic override */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(VkDevice dev, const VkSwapchainCreateInfoKHR* ci, const VkAllocationCallbacks* ac, VkSwapchainKHR* sc) {
    ensure_loaded();
    if (!orig_vkCreateSwapchainKHR) return VK_ERROR_INITIALIZATION_FAILED;
    VkSwapchainCreateInfoKHR tmp = *ci;
    return orig_vkCreateSwapchainKHR(dev, &tmp, ac, sc);
}

/* Present with FPS log */
VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pi) {
    ensure_loaded();
    static uint64_t frames=0; static time_t last=0;
    frames++;
    time_t now = time(NULL);
    if (now != last) {
        fprintf(stderr, "[XENO FPS] %lu FPS\n", frames);
        frames = 0; last = now;
    }
    if (orig_vkQueuePresentKHR) return orig_vkQueuePresentKHR(queue, pi);
    return VK_ERROR_INITIALIZATION_FAILED;
}

/* Expose minimal exports for loader */
__attribute__((visibility("default"))) PFN_vkVoidFunction vkGetInstanceProcAddr(void* instance, const char* name) {
    ensure_loaded();
    if (orig_vkGetInstanceProcAddr) return orig_vkGetInstanceProcAddr(instance, name);
    return NULL;
}
__attribute__((visibility("default"))) PFN_vkVoidFunction vkGetDeviceProcAddr(void* device, const char* name) {
    ensure_loaded();
    if (orig_vkGetDeviceProcAddr) return orig_vkGetDeviceProcAddr(device, name);
    return NULL;
}
