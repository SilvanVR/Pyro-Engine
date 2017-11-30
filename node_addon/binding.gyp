{

    'variables': {
        "ENGINE_ROOT%": "../vulkan-rendering-engine/vulkan-rendering-engine",  
        "ENGINE_BIN%": "../vulkan-rendering-engine/bin",  
        "ARCHITECTURE%": "x64", # Possible: x86, x64 (node-gyp: -arch=ia32 or -arch=??) x86 DOESNT WORK
        "CONFIGURATION%": "release" # Possible: debug, release (node-gyp: -debug or -release) DON'T FORGET TO CHANGE IN build.bat
    },
    
    "target_defaults":
    {
        "target_name": "renderer",
        "cflags": ["Wall", "-std=c++11"],

        "include_dirs": ["<!(node -e \"require('nan')\")",                    
                         "<(ENGINE_ROOT)/src",                      
                         "<(ENGINE_ROOT)/libs/include"],

        'Debug': {
            'defines': [ 'DEBUG', '_DEBUG', 'NOMINMAX' ]
        },

        'Release': {
            'defines': [ 'DEBUG', '_DEBUG', 'NOMINMAX' ]
        },

        "sources": 
        [ 
            "src_gyp/node_renderer.cc"
        ]        
    },

    "targets": [
        {    
            'conditions': [
                ['OS=="win"', {   

                    "libraries": [ # relative path is for libs the build-folder not the gyp-file              
                            "../<(ENGINE_BIN)/<(ARCHITECTURE)/<(CONFIGURATION)/vulkan-rendering-engine.lib",
                            "../<(ENGINE_ROOT)/libs/<(ARCHITECTURE)/<(CONFIGURATION)/glfw3.lib",
                            "../<(ENGINE_ROOT)/libs/<(ARCHITECTURE)/<(CONFIGURATION)/assimp-vc140-mt.lib",
                            "../<(ENGINE_ROOT)/libs/<(ARCHITECTURE)/<(CONFIGURATION)/freetype.lib",
                            "../<(ENGINE_ROOT)/libs/<(ARCHITECTURE)/<(CONFIGURATION)/FreeImageLib.lib",
                            "../<(ENGINE_ROOT)/libs/<(ARCHITECTURE)/vulkan-1.lib"
                            ],
                            
                    'configurations': {
                        'Debug': {
                            'msvs_settings': {
                                'VCCLCompilerTool': {
                                    'RuntimeLibrary': 3, # /MDd
                                }
                            }
                        },
                        'Release': {
                            'msvs_settings': {
                                'VCCLCompilerTool': {
                                    'RuntimeLibrary': 2, # /MD
                                }
                            }
                        }
                    }
                }]   
            ]
        }
    ]
}