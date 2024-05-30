file(GLOB_RECURSE FOC task_foc/*.c)
file(GLOB_RECURSE PERIO peripheral/*.c)
file(GLOB_RECURSE WIFI WIFI/*.c)
#file(GLOB_RECURSE DEMO demo_c/*.c)
idf_component_register(SRCS 
"lvgl_gui.c"
"main.c"
${FOC}
${PERIO}
${WIFI}
INCLUDE_DIRS "." 
"include_foc" 
#"include_demo"
 ) 
