#ifndef JSON_DEFINES_H_
#define JSON_DEFINES_H_

namespace Pyro
{
    #define ADD_CAMERA_COMPONENTS                   1 // Add movement + interaction script to the camera
    #define DEFAULT_WIDTH                           1280
    #define DEFAULT_HEIGHT                          720

    #define REPORT_WARNINGS                         false

    #define JSON_NAME_COLOR                         "color"
    #define JSON_NAME_COLOR_R_COMPONENT             "r"
    #define JSON_NAME_COLOR_G_COMPONENT             "g"
    #define JSON_NAME_COLOR_B_COMPONENT             "b"
    #define JSON_NAME_COLOR_A_COMPONENT             "a"

    #define JSON_NAME_TRANSFORM                     "transform"
    #define JSON_NAME_POSITION                      "position"
    #define JSON_NAME_X_COMPONENT                   "x"
    #define JSON_NAME_Y_COMPONENT                   "y"
    #define JSON_NAME_Z_COMPONENT                   "z"
    #define JSON_NAME_W_COMPONENT                   "w"
    #define JSON_NAME_SCALE                         "scale"
    #define JSON_NAME_ROTATION                      "rotation"

    #define JSON_NAME_CAMERA                        "camera"
    #define JSON_NAME_CAMERA_MODE                   "mode"
    #define JSON_NAME_CAMERA_MODE_PERSPECTIVE       "perspective"
    #define JSON_NAME_CAMERA_MODE_ORTHOGRAPHIC      "orthographic"
    #define JSON_NAME_CAMERA_FOV                    "fov"

    #define JSON_NAME_OBJECTS                       "objects"
    #define JSON_NAME_MATERIAL                      "material"
    #define JSON_NAME_MATERIALS                     "materials"
    #define JSON_NAME_MATERIAL_DIFFUSE              "diffuse"
    #define JSON_NAME_MATERIAL_NORMAL               "normal"
    #define JSON_NAME_MATERIAL_ROUGHNESS            "roughness"
    #define JSON_NAME_MATERIAL_METALLIC             "metallic"
    #define JSON_NAME_MATERIAL_AO                   "ao"
    #define JSON_NAME_MATERIAL_UVSCALE              "uvScale"

    #define JSON_NAME_LIGHTS                        "lights"
    #define JSON_NAME_LIGHT_INTENSITY               "intensity"
    #define JSON_NAME_LIGHT_TYPE                    "type"
    #define JSON_NAME_LIGHT_DIRECTIONAL             "directional"
    #define JSON_NAME_LIGHT_POINT                   "point"
    #define JSON_NAME_LIGHT_SPOT                    "spot"
    #define JSON_NAME_LIGHT_SPOT_FOV                "fov"
    #define JSON_NAME_LIGHT_ATTENUATION             "attenuation"
    #define JSON_NAME_LIGHT_DIRECTION               "direction"
    #define JSON_NAME_LIGHT_SHADOW_INFO             "shadowInfo"
    #define JSON_NAME_LIGHT_SHADOW_INFO_SIZE        "size"
    #define JSON_NAME_LIGHT_SHADOW_INFO_BLUR        "blur_strength"
    #define JSON_NAME_LIGHT_SHADOW_INFO_DISTANCE    "distance"

    #define JSON_NAME_SETTINGS                      "settings"
    #define JSON_NAME_IBL                           "ibl"
    #define JSON_NAME_ENABLED                       "enabled"
    #define JSON_NAME_ENVIRONMENT_MAP               "env_map"
    #define JSON_NAME_IRRADIANCE_SIZE               "irradiance_size"
    #define JSON_NAME_PREM_SIZE                     "prem_size"
    #define JSON_NAME_AMBIENT_INTENSITY             "ambient_intensity"
    #define JSON_NAME_SPECULAR_INTENSITY            "specular_intensity"
    #define JSON_NAME_SAMPLER                       "sampler"
    #define JSON_NAME_MAX_ANISO                     "max_aniso"
    #define JSON_NAME_FILTER                        "filter"
    #define JSON_NAME_MIPMAP_MODE                   "mipmap_mode"
    #define JSON_NAME_SRC                           "src"

    #define JSON_NAME_MODELS                        "models"
    #define JSON_NAME_MODEL                         "model"
    #define JSON_NAME_SKYBOX                        "skybox"
    #define JSON_NAME_CUBEMAP                       "cubemap"

    #define JSON_NAME_POST_PROCESSING               "post_processing"
    #define JSON_NAME_RES_MOD                       "resolution_mod"
    #define JSON_NAME_FXAA                          "fxaa"
    #define JSON_NAME_BLOOM                         "bloom"
    #define JSON_NAME_DEPTH_OF_FIELD                "doa"
    #define JSON_NAME_FOG                           "fog"
    #define JSON_NAME_LIGHT_SHAFTS                  "light_shafts"

    #define JSON_NAME_SHADERS                       "shaders"
    #define JSON_NAME_SHADER                        "shader"

    #define JSON_NAME_RESOLUTION                    "resolution"
    #define JSON_NAME_WIDTH                         "width"
    #define JSON_NAME_HEIGHT                        "height"

}


#endif // !JSON_DEFINES_H_

