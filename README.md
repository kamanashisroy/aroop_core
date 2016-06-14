![status](https://travis-ci.org/kamanashisroy/aroop_core.svg?branch=master)

aroop core
===========

This is complimentary C library for the aroop generated code.

Building
========

Aroop Core needs automake, libtool, pkg-config and check(unit-testing tool) installed.

Building core library binary is trivial. It is like `./autogen.sh;make;make install;`

```
a/aroop_core$ ./autogen.sh
a/aroop_core$ make
...
...
a/aroop_core$ ls
libaroop_core_static.a libaroop_core_debug.a libaroop_core_basic.o  libaroop_core.o 
a/aroop_core$ make install
```

It is possible to optimize the binary by configuring as following,

```
./autogen.sh CFLAGS='-O3' CXXFLAGS='-O3'
```

It is also possible to add debug symbols by adding -ggdb3 flag.

```
./autogen.sh CFLAGS='-ggdb3' CXXFLAGS='-ggdb3'
```

After build we shall get the object files(*libaroop_core_basic.o*,*libaroop_core.o*) and also an archive(*libaroop_core.a*). You may link them to your binary using `-laroop\_core` flag. Otherwise you can also link the object file(*libaroop_core.o*) with the binary.

Internals
==========

[virtual method table](http://en.wikipedia.org/wiki/Virtual_method_table)

Reading
========

- [The Basic Program/System Interface](http://ftp.gnu.org/old-gnu/Manuals/glibc-2.2.3/html_chapter/libc_25.html)
- [obstacks](http://www.gnu.org/software/libc/manual/html_node/Obstacks.html#Obstacks)
- [ZeroMQ](http://aosabook.org/en/zeromq.html), is a message passing library that uses less memory copy and less system call. It uses batching and memory pooling.

TASKS
=====

[tasks](TASKS.md)

