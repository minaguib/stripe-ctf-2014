CLUSTER_SERVERS:=h049 h050 h051 h052 h053 h054
PUBLIC_USERNAME=user-emk6s4lf

all: solve current-round
	./miner

clean:
	-rm -rf current-round solve

current-round:
	git clone lvl1-fm0yyrbl@stripe-ctf.com:current-round current-round

reset: current-round
	cd current-round && git fetch
	cd current-round && git reset --hard origin/master
	awk -F: '$$1=="${PUBLIC_USERNAME}" {score=++$$2; print $$1 ": " score} $$1!="${PUBLIC_USERNAME}" {print} END{if(!score) print "${PUBLIC_USERNAME}: " ++score}' current-round/LEDGER.txt > my_ledger.txt
	mv my_ledger.txt current-round/LEDGER.txt
	cd current-round && git add LEDGER.txt

solve: solve.c
	gcc -D_GNU_SOURCE -Wall -O3 -funroll-all-loops  -fomit-frame-pointer -lcrypto solve.c -o solve

# Cluster management:

monitor_cluster:
	while true; do ./monitor && $(MAKE) cluster_reset; done

cluster_reset: reset cluster_push cluster_kill_solve
	echo Cluster was reset

cluster_push:
	-parallel -j 0 rsync -a --delete --exclude solve . '{}':stripe-ctf/ ::: ${CLUSTER_SERVERS}

cluster_kill_solve:
	-parallel -j 0 ssh '{}' pkill solve ::: ${CLUSTER_SERVERS}

cluster_kill:
	-parallel -j 0 ssh '{}' pkill 'mine|solve' ::: ${CLUSTER_SERVERS}
