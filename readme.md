# Dead end

A big maze on the numworks calculator explorable in FPS view... Where your goal is to get out of this maze ! You are attacked by monsters fortunatly you have a weapon to fight back.

<img src=title_screen.png width=50%>

## BUILD IT

To build this app, you will need to install the [embedded ARM toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) and [Node.js](https://nodejs.org/en/). The C SDK for Epsilon apps is shipped as an npm module called [nwlink](https://www.npmjs.com/package/nwlink) that will automatically be installed at compile time.

You may need to install make

### LINUX

```shell
sudo apt install make
```

### WINDOWS

```shell
winget install ezwinports.make
```

### RUN IT

use the command below to send it to the calculator:

```shell
make PLATFORM=device run
```

### SIMULATOR

In order to use the simulator create a file called `sim_path.mak` then define the symbol *SIM* to be the path of your simulator.

```makefile
SIM := path/to/simulator
```

but for this to work you have to build the epsilon simulator see [there](https://github.com/numworks/epsilon) for more information. Once that is done use the command below to run in the simulator:

```shell
make PLATFORM=simulator run
```
