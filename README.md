# IFYT - Images for your terminal

Print images in any terminal supporting colours


### Building

```bash
make
```

### Running

```bash
ifyt [image path] [-t] [-v] [-h]
```

Example:

```bash
$ ifyt ./images/monkey.png -t
...
$
```

 - `-t` enable truecolor
 - `-v` output version
 - `-h` display usage

### Installing

```bash
make install
```

### Uninstalling

```bash
make uninstall
```

### Cleaning

```bash
make clean
```

### Todos

 - Add jpeg support
 - Clean everything up
 - etc.

### Dependencies

 - libpng

#### Install on Debian

```
sudo apt install libpng-dev
```

#### Install on CentOS

```
sudo yum install libpng-devel
```

### License

UNLICENSE

