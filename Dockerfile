FROM ubuntu:latest

RUN apt-get update

RUN apt-get install -y build-essential libgtk-3-dev

WORKDIR /app

COPY . .

RUN make

CMD ["./build/main"]

# docker build -t image_chess .
# docker run -e DISPLAY=host.docker.internal:0 -v /tmp/.X11-unix:/tmp/.X11-unix --name=program_chess image_chess