cmake_minimum_required(VERSION 2.8) 

PROJECT(EmbedHttpdTest)

INCLUDE_DIRECTORIES(../include) 

ADD_EXECUTABLE(EmbedHttpdTest EmbedHttpd.c)  
TARGET_LINK_LIBRARIES(EmbedHttpdTest EmbedHttpd)
