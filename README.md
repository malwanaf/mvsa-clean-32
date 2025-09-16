## Setup

1. Clone the repo
```
git clone https://github.com/malwanaf/mvsa-clean.git
cd mvsa-clean
```
2. Create a virtual environment
```
mvsa-clean $ uv venv (using uv)
mvsa-clean $ source .venv/bin/activate
mvsa-clean $ .venv/Scripts/activate (windows)
```
3. Install platformio
```
mvsa-clean $ uv pip install platformio
(or) mvsa-clean $ uv add platformio
```
4. Running the program
```
(venv) mvsa-clean $ pio run -t clean (clean run)
(venv) mvsa-clean $ pio run -t upload (upload directly to the connected MCU)
(venv) mvsa-clean $ pio run -e featheresp32 (generate firmware.elf for ESP32)
```
5. Using custom model
```
change the model.c and model.h in /src
change or remove the test_data.c and test_data.h if needed
change the user_main.c as intented
change the microvsa_config.h as needed
```
