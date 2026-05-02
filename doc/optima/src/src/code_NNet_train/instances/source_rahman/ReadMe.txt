Software Requirement-
Java Development Kit (JDK)


---------------------------------------
Linux OS
---------------------------------------
[Code tested on Ubuntu 10.04, OpenJDK Java 6 Web Start]

Execute the following command from terminal with path specified

java -jar NewJar1.jar

For example 
java -jar /home/effat/Desktop/NewJar1.jar


-------------------------------------
Windows
-------------------------------------
1. Double Click
OR
2. Run from command prompt-- Make current directory where the jar file resides.

 Then execute command : NewJar1.jar
______________________________________________________________

You can select data file and then select any method. Output will appear 
on textfield. You can also save the output results. 

For executing each method, you have to run the jar file each time.


Real data sets are assembled in Data folder.
 In Data folder, there are 3 subfolders containing real datasets of 3 algorithms. 
 
 data_SeqIc contains 100_input.txt which contains 100 inputs.
  
 The other datafiles starts with the 1st letter of protein family. For example
 p12.txt contains Protease datasets. 


 Run the file Browse.java. A GUI will appear.
 
 You will see 6 options--
   "Seq-Ic-LCS FA",
   "Seq-Ic-LCS CC", 
   "Seq-Ec-LCS FA", 
		          
    "Seq-Ec-LCS ZG", 
    "Str-Ec-LCS FA", 
    "Str-Ec-LCS HYA" 

  FA = Finite Automata , ZG, HYA, CC stands for DP algorithms 
  provided by respective authors.

  Select a data file with file browser and select the algorithm. You will see the outputs.
