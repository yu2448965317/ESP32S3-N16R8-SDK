import os
c_file = "idf_component_register(SRCS "
cur_dir = os.getcwd()+"\main"
makelist_file = cur_dir + "\CMakeLists.txt"
list_filename = os.listdir(cur_dir)
for i in list_filename:
        if ".c" in i :
            c_file = c_file + '"'+i+'"'+'\n'
c_file= c_file+'INCLUDE_DIRS ".")'
with open(makelist_file, 'w') as f:
         f.write(c_file)