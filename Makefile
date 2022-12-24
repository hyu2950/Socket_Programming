all: client.c serverM.c serverC.c serverCS.c serverEE.c
		gcc -o client client.c
		gcc -o serverM serverM.c
		gcc -o serverC serverC.c
		gcc -o serverCS serverCS.c
		gcc -o serverEE serverEE.c 

serverM:
		./serverM

serverC:
		./serverC

serverCS:
		./serverCS

serverEE:
		./serverEE

.PHONY: serverM serverC serverCS serverEE