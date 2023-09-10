
```statall;``` Report the status of all channels

```stat,<1-6>;``` Report the status of channel #1 

```configure,<confreg>,<value>``` Write the configuration value <value> into <confreg>

```getconfig,<confreg>``` Gets the configuration value of config register confreg

Goal: Commands should use as few bytes as possible

We have 6 Channels -> should fit inside 3 Bits

8 Bit format with error detection -> use UART parity bit

### Commands

| Command                  | B7  | B6  | B5  | B4  | B3   | B2    | B1    | B0    |
|--------------------------|-----|-----|-----|-----|------|-------|-------|-------|
| reset                    | 0   | 0   | 0   | 0   | 0    | 0     | 0     | 0     |
| statall                  | 0   | 0   | 1   | 0   | DNC  | DNC   | DNC   | DNC   |
| stat,addr                | 0   | 0   | 1   | 1   | DNC  | Addr2 | Addr1 | Addr0 |
| getconfig,confreg        | 0   | 1   | 0   | 0   | Reg3 | Reg2  | Reg1  | Reg0  |
| configure,confreg,value  | 1   | 0   | 0   | 0   | Reg3 | Reg2  | Reg1  | Reg0  |
|                          | V7  | V6  | V5  | V4  | V3   | V2    | V1    | V0    |

### Responses

| Response          | B7   | B6   | B5    | B4     | B3     | B2     | B1     | B0     |
|-------------------|------|------|-------|--------|--------|--------|--------|--------|
| error             | 0    | 0    | 0     | 0      | 0      | 0      | 0      | 0      |
| ack               | 0    | 0    | 0     | 1      | DNC    | DNC    | DNC    | DNC    |
| stat,addr         | 0    | 0    | 1     | 0      | stat   | Addr2  | Addr1  | Addr0  |
| statall           | 0    | 1    | stat5 | stat4  | stat3  | stat2  | stat1  | stat0  |
| getconfig,confreg | 1    | 0    | 0     | 0      | Reg3   | Reg2   | Reg1   | Reg0   |
|                   | V7   | V6   | V5    | V4     | V3     | V2     | V1     | V0     |

### Configuration Register

| Register Nr | R/W | Description      |
|-------------|-----|------------------|
| 0           | R   | Reserved         |
| 1           | R   | Reserved         |
| 2           | R   | Software version |
| 3           | R   | Hardware version |
| 4           | R/W | Channel 0 Config |
| 5           | R/W | Channel 1 Config |
| 6           | R/W | Channel 2 Config |
| 7           | R/W | Channel 3 Config |
| 8           | R/W | Channel 4 Config |
| 9           | R/W | Channel 5 Config |
| 10          | R   | Reserved         |
| 11          | R   | Reserved         |
| 12          | R   | Reserved         |
| 13          | R   | Reserved         |
| 14          | R   | Reserved         |
| 15          | R   | Reserved         |

