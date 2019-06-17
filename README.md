# CppND-System-Monitor

System Monitor Project in the C++ OOP. Based on [CppND-System-Monitor](https://github.com/udacity/CppND-System-Monitor).

## To setup and compile in Udacity Ubuntu workspace:

1. Clone repository into `/home/workspace/`
```
cd /home/workspace/
git clone https://github.com/wolegechu/System-Monitor
```

2. Install `ncurses` package
```
sudo apt-get install libncurses5-dev libncursesw5-dev
```
3. Compile and run
```
g++ -std="c++17" *.cpp -o ./system_monitor -lncurses
./system_monitor
```
4. just keep trying `./a.out` and it should work eventually.
