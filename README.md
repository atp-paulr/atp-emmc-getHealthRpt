# atp-emmc-getHealthRpt
This source code example uses the struct mmc_ioc_multi_cmd to read the Health data.  Using the struct mmc_ioc_multi_cmd reduces the chances of other processes interrupting the Vendor-Specific Commands (VSC) which might can cause the eMMC to respond unexpectedly. 
