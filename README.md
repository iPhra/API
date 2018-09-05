# simple-filesystem
This is the repository for the project "Prova finale di Algoritmi e Strutture Dati" (Final test of Algorithms and Data Structures) by Politecnico di Milano.  
It's a C implementation (libc only) of a simple Unix tree-shaped filesystem, saved in RAM.  
  

Inputs are given through command line, and consists of:  
-Creating an empty file in an existing directory  
-Creating an empty directory  
-Reading the content of a file in a given path  
-Overwriting the content of a file in a given path  
-Deleting an existing resource in the filesystem  
-Deleting a resource and all of its children nodes  
-Fiding the path of all the resources whose name is provided as input  
  

The time complexity of all the commands has to be at most O(l), with l being the length of the path. 
