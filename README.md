# EmbedHttpd
一个提供给嵌入式设备使用的Http服务器

# 目标
1. 较小的内存占用
2. 减少无必要的功能
3. 最重要的是：快速的http接口功能，嵌入式http服务器大部分使用cgi或者fastcgi，性能不强

# 特性
1. 使用 epoll 实现io复用
2. 使用 sendfile 来减少文件拷贝	
