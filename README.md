# ojito

A small eye to observe the world

## Setup

- Create a [Replicate API token](https://replicate.com/account/api-tokens)

- Add your token and Wifi credentials to [include/secrets.h](include/secrets.h) 

```cpp
#define REPLICATE_TOKEN "yourtoken"
#define SSID "yourssid"
#define PASSWORD "yourpass"
```

- Run convenience script. Installs dependencies, uploads program, and monitors serial output.

```bash
bash utils/Quickstart.sh
```
