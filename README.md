# 511-proj2-ABD
// github repo that is public, i will make this private once the project gets graded so future 511 students cant find it 
https://github.com/pqd5283/511-proj2-ABD# 

// setup / info
To start there is going to be 2 different files in the ZIP for this, there will be a nonblocking and a blocking.
Both will already be compiled so all you need to do is go into the build and run ./abd_client and ./abd_server 50051.
To recompile it just run make while in the build directory. (iflooking at code on github just switch branches to find the nonblocking) 
The testing stuff will run with whatever test I did last but then it will be free to input your own commands (read, write <value> and quit) 
To setup servers simply add the server to the config.txt with server# = <ip>:50051

// blocking design / code design as a whole
My code started with making the regular abd first and then the blocking version based off of it (which is why there is two sets of files because one is just the completed blocking and the other one is just the completed non-blocking sorry for it being like that and not nicely in one file). The way that the abd with blocking works is first we try to contact all the servers and get the lock from the server. Inside the server there is one lock available that is up for grabs, if its currently taken the client asking for it has to wait. Once a lock is granted (which there will be R/W of them) those are the only servers that get contacted from there and it's regular ABD which involves prodding the server and either getting the timestamp and value (Read) or just the timestamp (write). Then it does the writeback to the server with the new timestamp/value once the writeback is done the lock is relinquished and is up for grabs again. One interesting thing I would like to talk about is how I do the quorum stuff. Since I am spawning a thread for each server im trying to contact once we reach the quorum there will still be threads running once we continue to get whatever the max timestamp is, to combat this I moved the join to the very end of the function and lock the max values so that only currently returned values count as we dont care about what servers respond once the quorum reached. I toyed with the idea of cancelling the rest of the threads but got scared that might leak something so I did this work around instead. I also added deadlines to each rpc call from the client so that it could timeout. This could affect really negatively in the blocking as that if one client is really slow and has the locks a few other clients could time out and fail the quorum check.

Not mentioned in the requirements but for sources I basically used all the same sources from the last project as I kept a very similar structure with the rpc stuff. *Disclaimer as with last project I did use the vscode autofill code feature when it was presented to me but I would double check everytime that the code it wrote was what I wanted to put down*
I also feel like I have less comments than last project and im too tired to go put a bunch in so I hope what is there is sufficient! 

Thanks and have a good winter break! 

// testing
All testing was done via the functions I did in main, pretty much spawned threads to simulate clients so i didnt have to make a bunch of client's on AWS and have them all run at once which wouldve been a pain to setup. One caveat is that some latency / throughput drop could be considered because of the joining of threads inside the read/write which is required to return the threads in the tests as well as some prints I have to report back to the client user of their actions being successful. I also just tested 100 "Clients" all at once for all of them as I did not really understand what saturated meant but seeing the difference across servers with that many "Clients" all trying to do an operation on so few servers would suffice. All servers run on spot instances on T2.medium instances. 

-----BLOCKING TESTS


One Server Results
90 reads - 10 writes

Total time: 4.073302 sec
Total ops: 100
Reads: 90 (failures: 0)
Writes: 10 (failures: 0)
Average read latency:  2.725472 sec
Average write latency: 2.210713 sec
Throughput: 24.55 ops/sec

90 writes - 10 read 

Total time: 4.648410 sec
Total ops: 100
Reads: 10 (failures: 0)
Writes: 90 (failures: 0)
Average read latency:  2.986911 sec
Average write latency: 3.051273 sec
Throughput: 21.51 ops/sec


Three Server Results
90 reads - 10 writes 
Client Failed To Reach Quorum
This is likely because theres 100 clients trying to reach 3 servers and one grabs the lock for each
This makes sense! 

Client Failed To Reach Quorum
This is likely because theres 100 clients trying to reach 3 servers and one grabs the lock for each
This makes sense! 


Five Server Results
90 reads - 10 writes 
Client Failed To Reach Quorum
This is likely because theres 100 clients trying to reach 5 servers and one grabs the lock for each
This makes sense! 

10 reads - 90 writes
Client Failed To Reach Quorum
This is likely because theres 100 clients trying to reach 5 servers and one grabs the lock for each
This makes sense! 

-----NON BLOCKING TESTS 
One Server Results
90 reads - 10 writes
0.203529 sec
Total ops: 100
Reads: 90 (failures: 0)
Writes: 10 (failures: 0)
Average read latency:  0.174009 sec
Average write latency: 0.188573 sec
Throughput: 491.33 ops/sec

90 writes - 10 read 
0.192809 sec
Total ops: 100
Reads: 10 (failures: 0)
Writes: 90 (failures: 0)
Average read latency:  0.169487 sec
Average write latency: 0.155145 sec
Throughput: 518.65 ops/sec



Three Server Results
90 reads - 10 writes 
time: 0.502881 sec
Total ops: 100
Reads: 90 (failures: 0)
Writes: 10 (failures: 0)
Average read latency:  0.412694 sec
Average write latency: 0.399889 sec
Throughput: 198.85 ops/sec

10 reads - 90 writes 
time: 0.522935 sec
Total ops: 100
Reads: 10 (failures: 0)
Writes: 90 (failures: 0)
Average read latency:  0.466843 sec
Average write latency: 0.484279 sec
Throughput: 191.23 ops/sec


Five Server Results
90 reads - 10 writes
0.915847 sec
Total ops: 100
Reads: 90 (failures: 0)
Writes: 10 (failures: 0)
Average read latency:  0.847941 sec
Average write latency: 0.871006 sec
Throughput: 109.19 ops/sec

90 writes - 10 read 
time: 0.910525 sec
Total ops: 100
Reads: 10 (failures: 0)
Writes: 90 (failures: 0)
Average read latency:  0.853425 sec
Average write latency: 0.803433 sec
Throughput: 109.83 ops/sec

----
It makes sense that the nonblocking is VERY Fast compared to the blocking! 


