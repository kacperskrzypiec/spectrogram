cmake_minimum_required(VERSION 3.22)
project(MultiProject)

option(PRODUCTION_BUILD "Production build" OFF)

add_subdirectory(AsmLibrary)
add_subdirectory(CppLibrary)
add_subdirectory(Spectrogram)