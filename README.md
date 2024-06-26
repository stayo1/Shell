Task 1 - advanced programming

Requirements -
- make
- g++

Build -
- For install this tools run the following command in the terminal: make all

Get Started - 
./myshell
hello: date >> myfile
hello: cat myfile
Tue Jun  4 16:06:36 IDT 2024
hello: date –u >> myfile
hello: cat myfile
Tue Jun  4 16:06:36 IDT 2024
Tue Jun  4 13:07:24 UTC 2024
hello: wc -l < myfile
2
hello: prompt = hi:
hi:mkdir mkdir
hi:cd mkdir
Changed directory: /home/stav/advanced programming/task1/mkdir
hi:pwd
/home/stav/advanced programming/task1/mkdir
hi:touch file1 file2 file3
hi:!!
hi:ls
file1  file2  file3
hi:echo abc xyz
abc xyz
hi:ls
file1  file2  file3
hi:echo $?
0 
hi:ls no_such_file
ls: cannot access 'no_such_file': No such file or directory
hi:echo $?
2 
hi:ls no_such_file 2> file
You typed Control-C! 
cat > colors.txt
blue
black
red
red
green
blue
green
red
red
blue
hi:cat colors.txt
blue
black
red
red
green
blue
green
red
red
blue
hi:cat colors.txt | cat | cat | cat
blue
black
red
red
green
blue
green
red
red
blue
hi:sort colors.txt | uniq -c | sort -r | head -3
      4 red
      3 blue
      2 green
hi:quit

Authors - 
- moshe nahshon
- stav sharon 