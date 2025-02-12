idf_component_register(
                         SRCS 
                              "./main.cpp" 
                              "./PWR_Key/PWR_Key.c"
                              "./FluidSim/Scene.cpp"
                              "./QMI8658/QMI8658.cpp"

                         INCLUDE_DIRS 
                              "./PWR_Key"
                              "./FluidSim"
                              "./QMI8658"
                              "."
                       )