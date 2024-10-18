# Создать образ на основе базового слоя gcc (там будет ОС и сам компилятор).
# 11.3 — используемая версия gcc.
FROM gcc:11.3 as build

# Выполнить установку зависимостей внутри контейнера.
RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip3 install conan==1.*

# копируем conanfile.txt в контейнер и запускаем conan install
COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. -s compiler.libcxx=libstdc++11 -s build_type=Debug

# Скопировать файлы проекта внутрь контейнера
COPY ./src /app/src
COPY ./tests /app/tests
COPY CMakeLists.txt conanfile.txt /app/ 

# новая команда для сборки сервера:
RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && \
    cmake --build . 

# second container
FROM ubuntu:22.04 as run

# add user to container
RUN groupadd -r www && useradd -r -g www www
USER www

# copy config and app
COPY --from=build /app/build/game_server /app/
COPY ./static /app/static
COPY ./data /app/data

# auto start app
ENTRYPOINT ["/app/game_server", "-c", "/app/data/config.json", "-w", "/app/static", "--state-file=/tmp/test.bin"]
