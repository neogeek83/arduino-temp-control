Same as TempReadOut, except uses interupts for the temp reading.

Both support IR remote (which also uses interrupts). They do sometimes stomp on each other(this version handles it better).

Source for DHT11 interrupts: https://github.com/niesteszeck/idDHT11