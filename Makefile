
CXX = g++
CXXFLAGS = -std=c++17 -O2
SRC_DIR = src
BIN_DIR = /usr/local/bin
TARGET = bubblescli

all: $(TARGET)

$(TARGET): $(SRC_DIR)/bubblecli.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC_DIR)/bubblecli.cpp

install: $(TARGET)
	sudo cp $(TARGET) $(BIN_DIR)/$(TARGET)
	@mkdir -p ~/.config/bubblecli
	@cp default_config.ini ~/.config/bubblescli/config.ini 2>/dev/null || true
	@echo " Installed as '$(BIN_DIR)/$(TARGET)'"
	@echo " Config created at ~/.config/bubblescli/config.ini (or already exists)"

uninstall:
	sudo rm -f $(BIN_DIR)/$(TARGET)
	@rm -rf ~/.config/bubblescli
	@echo "🗑️ Uninstalled '$(TARGET)' and removed config"

clean:
	rm -f $(TARGET)
