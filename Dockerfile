FROM trzeci/emscripten:latest
WORKDIR /src
RUN apt-get update 
RUN apt-get dist-upgrade -y
RUN apt-get install -y libboost-all-dev
RUN apt-get install -y automake autoconf libtool
CMD ["/bin/bash"]