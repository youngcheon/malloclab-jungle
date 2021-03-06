# malloc-lab-jungle
## ๐ก ์ง๋ ์ฃผ์ ์ฌ์ฉํ๋ malloc์ ์ด๋ฒ ์ฃผ์๋ ๋ง๋ค์ด ๋ณด์์

โ๋์  ๋ฉ๋ชจ๋ฆฌ ํ ๋นโ ๋ฐฉ๋ฒ์ ์ง์  ๊ฐ๋ฐํ๋ฉด์ ๋ฉ๋ชจ๋ฆฌ, ํฌ์ธํฐ ๊ฐ๋์ ์ต์ํด์ง ์ ์๋ ๊ณผ์ ์๋๋ค. ๋ฉ ์ฝ๋๋ฅผ ์ง์  ์์ ํ๊ณ , ์ฑ์  ํ๋ก๊ทธ๋จ์ ์คํํ๋ฉด์ โ๋ด ์ ์โ๋ฅผ ์ ์ ์์ต๋๋ค.

โ ์ฆ, ๋๋ง์ malloc, realloc, free ํจ์๋ฅผ ๊ตฌํํ๋ ๊ฒ!

implicit, explicit, seglist ๋ฑ ์ฌ๋ฌ ๋ฐฉ๋ฒ์ด ์์ง๋ง, ์ผ๋จ implicit ๋ฐฉ๋ฒ๋ถํฐ ๊ตฌํํด ๋ณด๊ฒ ์ต๋๋ค. ์ฌ์ ๊ฐ ๋๋ฉด explicit, seglist, buddy system ๊น์ง๋ ๋์ ํด๋ณด์ธ์.
์ปดํจํฐ ์์คํ ๊ต์ฌ์ 9.9์ฅ์ ์ฐฌ์ฐฌํ ์ฝ์ด๊ฐ๋ฉฐ ์งํํ์ธ์. ๊ต์ฌ์ ์ฝ๋๋ฅผ ์ดํดํ๊ณ  ์ฎ๊ฒจ์จ์ ์ ๋์ํ๋๋ก ์คํํด๋ณด๋ ๊ฒ์ด ์์์๋๋ค!
https://github.com/SWJungle/malloclab-jungle ์ ๋ด์ฉ๋๋ก ์งํํฉ๋๋ค.
โ์ปดํจํฐ ์์คํโ์ฑ์ malloclab-handout์ ๋ด์ฉ์ ๋ฐ๋ผํฉ๋๋ค.
mm.c๋ฅผ ๊ตฌํํ๊ณ  mdriver๋ก ์ฑ์ (ํ์คํธ) ํฉ๋๋ค.

## ์งํ๋ฐฉ๋ฒ
`make` ํ `./mdriver` ๋ฅผ ์คํํ๋ฉด out of memory ์๋ฌ ๋ฐ์
์ฑ์ ์๋ implicit list ๋ฐฉ์๋๋ก malloc์ ๊ตฌํํด์ ํด๋น ์๋ฌ๋ฅผ ์์ ๊ธฐ
์ดํ (์๊ฐ์ด ๋๋ค๋ฉด) explicit list์ seglist ๋ฑ์ ํ์ฉํด ์ ์๋ฅผ ๋์ด๊ธฐ
Tip: `./mdriver -f traces/binary2-bal.rep` ์ ๊ฐ์ด ํน์  ์ธํธ๋ก๋ง ์ฑ์  ํ  ์ ์๋ค.

์ถ๊ฐ์ฌํญ
`python3 auto-grade.py` ๋ก ์๋์ฑ์  ํ  ์ ์๋ค.

## ๋ง๋ก๋ฉ ๊ณผ์  ์ค๋ช
http://csapp.cs.cmu.edu/3e/malloclab.pdf
์ถ์ฒ: CMU (์นด๋ค๊ธฐ๋ฉ๋ก )
## ์ฐธ๊ณ ์๋ฃ

http://csapp.cs.cmu.edu/3e/labs.html
CMU ๊ฐ์์๋ฃ ์ ์ฒด


## GOAL

implicit ๋ฐฉ๋ฒ์ ์ด์ฉํด `./mdriver` ๊ฐ ์ ์ ์๋ํ๋๋ก ์ฝ๋๋ฅผ ์์ฑํ๊ธฐ


์๋๋ ์๋ณธ READMEํ์ผ์๋๋ค.
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
