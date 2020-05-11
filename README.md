# OS_Virtual_Memory

[COPYRIGHT]

이 프로젝트의 모든 저작권은 숭실대학교 운영체제 LAB에있습니다.
이 프로젝트는 컴퓨터학부 운영체제 수업의 6번째 프로젝트였습니다.

All copyright is belong to Soongsil University OS Lab and this project was sixth subject of OS class.

[OVERVIEW]

과제의 주요 목표는 Page Allocator를 구현하는 것입니다. 이 과제를 수행하기 위해선 '가상 메모리'에 관한 개념과 '페이징'에 관한 개념이 필요합니다. 가상 메모리란
물리적 메모리(실제 메모리)를 추상화하여 실존하는 메모리보다 더 큰 주소 공간을 지원할 수 있도록 하는 것입니다. 이 주소 공간은 'Page'라는 (대체로) 4KB의 메모리 
조각을 가리키는데 일종의 포인터와 같은 역할을 수행합니다. 32bit 컴퓨터의 경우 4GB의 가상 주소 공간을 가지는데 4KB (페이지 조각 하나의 크기) X 4GB = 16TB가 
나오는 것을 보듯이 가상 메모리 주소 공간과 실제 메모리간 1:1 대응은 불가능함을 알 수 있습니다. 

이 과제는 가상 메모리 주소 공간과 페이징 기법에 대한 이해를 기반으로 Page Allocator를 구현하는 것을 목표로 합니다. 가상 메모리는 'Kernel Pool'과 'User 
Pool'로 나눠서 관리하는데 bitmap을 이용하여 사용 가능한 page frame을 관리해야 합니다. 이를 위한 과제 수행 내용은 크게 3가지입니다.

32bit 기반 우분투 16.04에서 실행시킬 수 있습니다. 또한, 연습용 OS인 SSUOS의 실제 메모리는 128MB이며 Page Frame 한개의 크기는 4KB입니다.
더 자세한 명세는 pdf파일로 소스파일에 첨부하였습니다.

The main object of this project is to make a 'Page Allocator'. To make this project, We need to understand what the virtual 
memory is and what the paging is. Virtual memory is to offer much bigger address space than real existing memory space by 
abstracting physical memory. This address space is called 'Page' that points 4KB memory pieace. If we know the definitions of 
the virtual memory and the paging, we can understand that it is impossible to match one to one between virtual memory and 
physical memory.

The object of this project is to make 'Page Allocator' based on understanding of virtual memory space and paging method. The virtual memory is splited into 'Kernel Pool' and 'User Pool' that we have to manage an usable page frame by bitmap. The main
aims were written at [OBJECTS].

If you want to execute this source code on your computer, you have to run this software on 32bit Ubuntu Linux 16.04. In 
addition, The size of physical memory of the 'SSUOS' which is OS for practice is 128MB and the size of one page frame is 4KB.


[OBJECTS]

1. kernel pool과 user pool의 구현 : palloc.c의 init_palloc()에 구현
   Make kernel pool and user pool : I wrote them in 'init_palloc()' of palloc.c

2. palloc.c의 메모리 할당 및 삭제 함수 구현 : palloc.c의 palloc_get_multiple_page() / palloc_free_multiple_page()에 구현
   Make memory allocating function and deleting function in palloc.c : 
   -> 'palloc_get_multiple_page()' and 'palloc_free_multiple_page()'

3. 프로세스 생성 시 page directory 및 page table 구현 : paging.c의 pd_create() / pd_copy() / pt_copy()에 구현
   Make page directory and page table when the process is made : 'pd_create()','pd_copy()',and 'pt_copy()' of paging.c
