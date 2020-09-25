# Adding devices to netlist

There are basically two kind of devices you can add to netlist:

- coded devices. These can be found in the devices and analog folder. Typically
these are logic or elementary (i.e. resistor, capacitor) devices.
- netlist language devices. These are found in the macro folder structure.
Typically these are dip layouts or devices consisting of elementary devices. A good 
example are operational amplifier models or VCOs.

In prior netlist releases (<=0.13.0) it was cumbersome to add devices. You had to make
sure to add references and code in a number of different locations.

All necessary header files are now created automatically.

## Stand alone compilation

### Coded devices

Just add your device code (e.g. nld_device.cpp) into the devices folder.

Switch to the `build` directory

`cd build`

and type

`make generated`

to create all the infrastructure code.

### Macro devices

Place the device code (e.g. nlmod_device.cpp) into macro/modules. 

Switch to the `build` directory

`cd build`

and type

`make generated`

to create all the infrastructure code.

## Visual Studio build

You need to add the files manually to the Visual Studio solution.

## MAME integration

In addition to the the steps above you have to add your code additions to the
`scripts/src/netlist.lua` file.

Make sure to

```sh
cd src/lib/netlist/build
make generated
```

to (re)create the necessary files in the `generated` directory.

## Background

### What does `make generated` do?

`make generated` calls three python scripts:

- `create_devinc.py`
- `create_lib_entries.py`
- `create_modules.py`

which create the following files in the `generated` directory:

- `nld_devinc.h`
- `lib_entries.hxx`
- `nlm_modules_lib.cpp`

The information contained in these files in the past had to be included in various places
and files previously. Since the information can be extracted programmatically the
manual process now was replaced.
