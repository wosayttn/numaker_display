# **nuamker_display**

This repository contains the display drivers for the NuMaker development board from Nuvoton.

## Clone Repositories

Clone the main nuamker_display repository and the necessary git modules separately. For example, you only need the **bsp/m55m1** git submodules, execute the following command for cloning in git bash console.

```bash
git clone https://github.com/wosayttn/nuamker_display
cd lv_port_nuvoton
git submodule update --progress --init -- "bsp/m55m1"
```

or, clone lv_port_nuvoton with recurse-sumbmodules option for cloning all repositories directly.

```bash
git clone --recurse-submodules https://github.com/wosayttn/nuamker_display
```
