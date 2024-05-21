# SPIDER - C++ web (services) toolkit

## Quick start

Please checkout the code in [example_controller.cpp](examples/example_controller/example_controller.cpp).
It demonstrates the capabilities that this library provides.

To test this application, run it as follows:
```shell
DOCROOT=/tmp/myfiles
mkdir -p ${DOCROOT}
cd ${CMAKE_BINARY_DIR}/zoo/spider/examples/example_controller # fill in CMAKE_BINARY_DIR according to your build setup
./spider_example_controller 0.0.0.0 8080 ${DOCROOT} 1
# argv[1] is the listen address
# argv[2] is the listen port
# argv[3] is the doc root for file downloads
# argv[4] is the number of threads. 1 is sufficient for functional testing.
```

To test the file download endpoint, create some file in the doc root that you set in the command line above
```shell
cat > ${DOCROOT}/hello.txt <<EOF
Hello World!
EOF
```

Now get it. Assuming the server is listening on port 8080:
```shell
# Files in the doc root are served by the path /files/<FILE_PATH>
curl http://localhost:8080/files/hello.txt
```

By default, the server will track file downloads, i.e. it will check if the file was completely read.
This does not guarantee that the client receives the file completely, but it can tell you if a file download was interrupted.
To disable tracking, use the `track=0` query parameter:
```shell
curl http://localhost:8080/files/hello.txt?track=0
```

The output should be the same, but in the server logging, there will no longer be a message like:
```text
Downloaded file of 13 bytes was read completely
```

Futhermore, the server exposes a basic REST API.

* GET /api/customer/{id}
  ```shell
  curl http://localhost:8080/api/customer/42 \
  	-H "x-api-key: the_api_key" \
  && echo
  ```
* POST /api/customer with JSON payload
  ```shell
  curl -X POST http://localhost:8080/api/customer \
  	-H "Content-Type: application/json" \
  	-d '{"id":42, "name":"John Doe"}' \
  && echo
* __ANY__ /api/noop, does nothing, always succeeds
  ```shell
  curl -X PUT http://localhost:8080/api/noop \
  	-H "Content-Type: application/text" \
  	-d 'Hello' \
  && echo
  ```
* __ANY__ /api/fail, does nothing, always fails
  ```shell
  curl http://localhost:8080/api/fail \
  && echo
  ```

## Motivation

While using Boost Beast in some projects, I found myself copying considerable amounts of code from one project to the next.
Also, handling web requests requires too much hand written code for my taste.
So I started this library to make my life easier. Perhaps it can make your life easier as well ;-)

