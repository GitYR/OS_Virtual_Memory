# OS_Virtual_Memory

[COPYRIGHT]

이 프로젝트의 모든 저작권은 숭실대학교 운영체제 LAB에있습니다.
이 프로젝트는 컴퓨터학부 운영체제 수업의 6번째 프로젝트였습니다.
저는 교수님이 의도하셨던 주요 함수들을 채우는 식으로 과제를 수행했습니다.

[OVERVIEW]

과제의 주요 목표는 Page Allocator를 구현하는 것입니다. 이 과제를 수행하기 위해선 '가상 메모리'에 관한 개념과 '페이징'에 관한 개념이 필요합니다. 가상 메모리란
물리적 메모리(실제 메모리)를 추상화하여 실존하는 메모리보다 더 큰 주소 공간을 지원할 수 있도록 하는 것입니다. 이 주소 공간은 'Page'라는 (대체로) 4KB의 메모리 
조각을 가리키는데 일종의 포인터와 같은 역할을 수행합니다. 32bit 컴퓨터의 경우 4GB의 가상 주소 공간을 가지는데 4KB (페이지 조각 하나의 크기) X 4GB = 16TB가 
나오는 것을 보듯이 가상 메모리 주소 공간과 실제 메모리간 1:1 대응은 불가능함을 알 수 있습니다. 

이 과제는 가상 메모리 주소 공간과 페이징 기법에 대한 이해를 기반으로 Page Allocator를 구현하는 것을 목표로 합니다. 가상 메모리는 'Kernel Pool'과 'User 
Pool'로 나눠서 관리하는데 bitmap을 이용하여 사용 가능한 page frame을 관리해야 합니다. 이를 위한 과제 수행 내용은 크게 3가지입니다.

32bit 기반 우분투 16.04에서 실행시킬 수 있습니다. 또한, 연습용 OS인 SSUOS의 실제 메모리는 128MB이며 Page Frame 한개의 크기는 4KB입니다.
더 자세한 명세는 pdf파일로 소스파일에 첨부하였습니다.

[OBJECTS]

1. kernel pool과 user pool의 구현 : palloc.c의 init_palloc()에 구현

2. palloc.c의 메모리 할당 및 삭제 함수 구현 : palloc.c의 palloc_get_multiple_page() / palloc_free_multiple_page()에 구현

3. 프로세스 생성 시 page directory 및 page table 생성 수정 구현 : paging.c의 pd_create() / pd_copy() / pt_copy()에 구현
