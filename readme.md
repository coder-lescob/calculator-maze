# Maze

A big maze on the numworks calculator...

## BUILD IT

To build this sample app, you will need to install the [embedded ARM toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) and [Node.js](https://nodejs.org/en/). The C SDK for Epsilon apps is shipped as an npm module called [nwlink](https://www.npmjs.com/package/nwlink) that will automatically be installed at compile time.

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

```make
SIM := path/to/simulator
```

but for this to work you have to build the epsilon simulator see [there](https://github.com/numworks/epsilon) for more information
