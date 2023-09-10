
```statall;``` Report the status of all channels

```stat,<1-6>;``` Report the status of channel #1 

```ver;``` Report the version

```configure,<confreg>,<value>``` Write the configuration value <value> into <confreg>

```getconfig,<confreg>``` Gets the configuration value of config register confreg

Goal: Commands should use as few bytes as possible

We have 6 Channels -> should fit inside 3 Bits

8 Bit format with error detection -> use UART parity bit

### Commands

| Command                  | B7  | B6  | B5  | B4  | B3   | B2    | B1    | B0    |
|--------------------------|-----|-----|-----|-----|------|-------|-------|-------|
| reset                    | 0   | 0   | 0   | 0   | 0    | 0     | 0     | 0     |
| ver                      | 0   | 0   | 0   | 1   | DNC  | DNC   | DNC   | DNC   |
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
| statall           | 1    | 0    | stat5 | stat4  | stat3  | stat2  | stat1  | stat0  |
| getconfig,confreg | 0    | 1    | 0     | 0      | Reg3   | Reg2   | Reg1   | Reg0   |
|                   | V7   | V6   | V5    | V4     | V3     | V2     | V1     | V0     |
| ver               | 0    | 1    | 0     | 1      | Maj1   | Maj0   | Min4   | Min3   |
|                   | Min2 | Min1 | Min0  | Patch4 | Patch3 | Patch2 | Patch1 | Patch0 |

