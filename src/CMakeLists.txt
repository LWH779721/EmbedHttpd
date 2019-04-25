cmake_minimum_required(VERSION 2.8) 

PROJECT(EmbedHttpd)
 
INCLUDE_DIRECTORIES(../include) 

AUX_SOURCE_DIRECTORY(. EmbedHttpd)

ADD_LIBRARY (EmbedHttpd SHARED ${EmbedHttpd})
