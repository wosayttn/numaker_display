# **numaker_display**

This repository contains the display drivers for the NuMaker development board from Nuvoton.

## Clone Repositories

Clone the main numaker_display repository and the necessary git modules separately. For example, you only need the **bsp/m55m1** git submodules, execute the following command for cloning in git bash console.

```bash
git clone https://github.com/wosayttn/numaker_display
cd numaker_display
git submodule update --progress --init -- "bsp/m55m1"
```

or, clone numaker_display with recurse-submodules option for cloning all repositories directly.

```bash
git clone --recurse-submodules https://github.com/wosayttn/numaker_display
```
