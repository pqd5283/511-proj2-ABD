# 511-proj2-ABD
// github repo that is public, i will make this private once the project gets graded so future 511 students cant find it 
https://github.com/pqd5283/511-proj2-ABD# 

// setup / info
To start there is going to be 2 different files in the ZIP for this, there will be a nonblocking and a blocking.
Both will already be compiled so all you need to do is go into the build and run ./abd_client and ./abd_server 50051.
To recompile it just run make while in the build directory.
The testing stuff will run with whatever test I did last but then it will be free to input your own commands (read, write <value> and quit) 

// blocking design / code design as a whole
My code started with making the regular abd first and then the blocking version based off of it (which is why there is two sets of files because one is just the completed blocking and the other one is just the completed non-blocking sorry for it being like that and not nicely in one file). The way that the abd with blocking works is first we try to contact all the servers and get the lock from the server. Inside the server there is one lock available that is up for grabs, if its currently taken the client asking for it has to wait. Once a lock is granted (which there will be R/W of them) those are the only servers that get contacted from there and it's regular ABD which involves prodding the server and either getting the timestamp and value (Read) or just the timestamp (write). Then it does the writeback to the server with the new timestamp/value once the writeback is done the lock is relinquished and is up for grabs again. One interesting thing I would like to talk about is how I do the quorum stuff. Since I am spawning a thread for each server im trying to contact once we reach the quorum there will still be threads running once we continue to get whatever the max timestamp is, to combat this I moved the join to the very end of the function and lock the max values so that only currently returned values count as we dont care about what servers respond once the quorum reached. I toyed with the idea of cancelling the rest of the threads but got scared that might leak something so I did this work around instead. I also added deadlines to each rpc call from the client so that it could timeout. This could affect really negatively in the blocking as that if one client is really slow and has the locks a few other clients could time out and fail the quorum check.

Not mentioned in the requirements but for sources I basically used all the same sources from the last project as I kept a very similar structure with the rpc stuff. *Disclaimer as with last project I did use the vscode autofill code feature when it was presented to me but I would double check everytime that the code it wrote was what I wanted to put down*

// testing
