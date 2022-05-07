# 탐험준비 - Week06 Malloc lab
## 💡 지난 주에 사용했던 malloc을 이번 주에는 만들어 보아요

> ‘동적 메모리 할당’ 방법을 직접 개발하면서 메모리, 포인터 개념에 익숙해질 수 있는 과제입니다. 랩 코드를 직접 수정하고, 채점 프로그램을 실행하면서 ‘내 점수’를 알 수 있습니다.

→ 즉, 나만의 malloc, realloc, free 함수를 구현하는 것!

>implicit, explicit, seglist 등 여러 방법이 있지만, 일단 implicit 방법부터 구현해 보겠습니다. 여유가 되면 explicit, seglist, buddy system 까지도 도전해보세요.
컴퓨터 시스템 교재의 9.9장을 찬찬히 읽어가며 진행하세요. 교재의 코드를 이해하고 옮겨써서 잘 동작하도록 실행해보는 것이 시작입니다!
https://github.com/SWJungle/malloclab-jungle 의 내용대로 진행합니다.
‘컴퓨터 시스템’책의 malloclab-handout의 내용을 따라합니다.
mm.c를 구현하고 mdriver로 채점(테스트) 합니다.

## 진행방법
`make` 후 `./mdriver` 를 실행하면 out of memory 에러 발생
책에 있는 implicit list 방식대로 malloc을 구현해서 해당 에러를 없애기
이후 (시간이 된다면) explicit list와 seglist 등을 활용해 점수를 높이기
Tip: `./mdriver -f traces/binary2-bal.rep` 와 같이 특정 세트로만 채점 할 수 있다.

## 말록랩 과제 설명
http://csapp.cs.cmu.edu/3e/malloclab.pdf
출처: CMU (카네기멜론)
## 참고자료

http://csapp.cs.cmu.edu/3e/labs.html
CMU 강의자료 전체


## GOAL

> implicit 방법을 이용해 ./mdriver 가 정상 작동하도록 코드를 완성하기


아래는 원본 README파일입니다.
~~~

#####################################################################
# CS:APP Malloc Lab
# Handout files for students
#
# Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
# May not be used, modified, or copied without permission.
#
######################################################################

***********
Main Files:
***********

mm.{c,h}	
	Your solution malloc package. mm.c is the file that you
	will be handing in, and is the only file you should modify.

mdriver.c	
	The malloc driver that tests your mm.c file

short{1,2}-bal.rep
	Two tiny tracefiles to help you get started. 

Makefile	
	Builds the driver

**********************************
Other support files for the driver
**********************************

config.h	Configures the malloc lab driver
fsecs.{c,h}	Wrapper function for the different timer packages
clock.{c,h}	Routines for accessing the Pentium and Alpha cycle counters
fcyc.{c,h}	Timer functions based on cycle counters
ftimer.{c,h}	Timer functions based on interval timers and gettimeofday()
memlib.{c,h}	Models the heap and sbrk function

*******************************
Building and running the driver
*******************************
To build the driver, type "make" to the shell.

To run the driver on a tiny test trace:

	unix> mdriver -V -f short1-bal.rep

The -V option prints out helpful tracing and summary information.

To get a list of the driver flags:

	unix> mdriver -h
~~~
