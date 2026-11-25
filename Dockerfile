# syntax=docker/dockerfile:1
FROM ubuntu:24.04 AS build
RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential cmake qt6-base-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)" \
 && ctest --test-dir build --output-on-failure

FROM ubuntu:24.04
RUN apt-get update \
 && apt-get install -y --no-install-recommends libqt6widgets6t64 libqt6concurrent6t64 fonts-dejavu-core \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --create-home --shell /usr/sbin/nologin pwgen
COPY --from=build /src/build/PasswordGenerator /usr/local/bin/PasswordGenerator
COPY --from=build /src/build/password_generator_multithreaded /usr/local/bin/password_generator_multithreaded
USER pwgen
ENV QT_QPA_PLATFORM=xcb
CMD ["password_generator_multithreaded"]
