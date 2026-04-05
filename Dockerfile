FROM ghcr.io/steven-darwin/scientific_computing/codebase:latest as preparation

WORKDIR /home/scientific_computing
RUN mkdir simulation_visualizer
WORKDIR ./simulation_visualizer
RUN mkdir lib

FROM scratch as dependency

# Resolve vcpkg package
WORKDIR /home/scientific_computing/simulation_visualizer/lib
COPY ./lib/vcpkg.json ./lib/vcpkg-configuration.json ./
RUN vcpkg install

FROM scratch as final

WORKDIR /home/scientific_computing/simulation_visualizer
COPY ./CMakeLists.txt ./CMakePresets.json ./CMakeUserPresets.json ./
RUN mkdir include
COPY ./include ./include
RUN mkdir src
COPY ./src .src
RUN mkdir assets
COPY ./assets ./assets
RUN mkdir config
COPY ./config ./config
RUN mkdir scripts
COPY ./scripts ./scripts