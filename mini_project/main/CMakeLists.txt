# 这个命令是用来向当前组件注册内容的
# SRCS是该命令的一个参数名 字面意思为源文件 也就是C文件，当前内容为 "main.c"
# INCLUDE_DIRS是该命令的第二个参数名 字面意思为要include的目录 也就是存放头文件的路径
#    当前内容为 ".", 指的是main目录自己。
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
)
