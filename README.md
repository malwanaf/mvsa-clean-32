##Setup

1. Clone the repo
2. Create a virtual environment
```
uv venv (using uv)
.venv/bin/activate
.venv/Scripts/activate (windows)
```
3. Install platformio
```
uv pip install platformio
(or) uv add platformio
```
4. Using custom model
```
change the model.c and model.h in /src
change or remove the test_data.c and test_data.h if needed

change the user_main.c as intented
change the microvsa_config.h as needed
```
