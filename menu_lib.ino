#include "Wire.h"

#define LCD_ADDR 0x27
#define LCD_SDA 40
#define LCD_SCL 41

#define UPBtn 8
#define DOWNBtn 3
#define OKBtn 46

class LiquidCrystal {
public:

  LiquidCrystal() {}
  LiquidCrystal(uint8_t Address, uint8_t SDA, uint8_t SCL) {
    _Address = Address;
    _SDA = SDA;
    _SCL = SCL;

    Wire.begin(_SDA, _SCL, 1000000);  // آغاز ارتباط I2C با سرعت بالا
    delay(100);
  }

  void begin(uint8_t line = 2, uint8_t column = 16) {
    _line = line;
    _column = column;
    lcdSendCommand(0X0C);  // روشن کردن نمایشگر و خاموش کردن مکان‌نما
    delay(50);
    lcdSendCommand(0X28);  // تنظیم نمایشگر دو سطری با بلوک‌های 5x8
    delay(100);

    clear();  // پاک کردن نمایشگر
  }

  // نوشتن متن روی نمایشگر
  void write(String data) {
    int i = 0;
    while (data[i] != '\0') {
      lcdSendData(data[i]);
      i++;
    }
  }

  // پاک کردن نمایشگر
  void clear() {
    lcdSendCommand(0X01);
    delay(1);
  }

  // تنظیم مکان‌نما
  void setCursor(int y, int x) {
    uint8_t address;
    if (x == 0) {
      address = y;  // آدرس سطر اول
    } else {
      address = 0x40 + y;  // آدرس سطر دوم
    }
    lcdSendCommand(0x80 | address);  // ارسال آدرس به DDRAM
  }

private:
  uint8_t _SDA;
  uint8_t _SCL;
  uint8_t _Address;

  uint8_t _line;
  uint8_t _column;

  // ارسال فرمان به نمایشگر
  void lcdSendCommand(byte command) {
    Wire.beginTransmission(_Address);
    Wire.write(command & 0xF0 | 0x0C);         // ارسال نیم‌بایت اول
    Wire.write(command & 0xF0 | 0x08);         // ارسال نیم‌بایت اول بدون فعال‌سازی
    Wire.write((command << 4) & 0xF0 | 0x0C);  // ارسال نیم‌بایت دوم
    Wire.write((command << 4) & 0xF0 | 0x08);  // ارسال نیم‌بایت دوم بدون فعال‌سازی
    Wire.endTransmission();
    delay(5);
  }

  // ارسال داده به نمایشگر
  void lcdSendData(byte data) {
    Wire.beginTransmission(_Address);
    Wire.write(data & 0xF0 | 0x0D);         // ارسال نیم‌بایت اول برای داده
    Wire.write(data & 0xF0 | 0x09);         // ارسال نیم‌بایت اول بدون فعال‌سازی
    Wire.write((data << 4) & 0xF0 | 0x0D);  // ارسال نیم‌بایت دوم برای داده
    Wire.write((data << 4) & 0xF0 | 0x09);  // ارسال نیم‌بایت دوم بدون فعال‌سازی
    Wire.endTransmission();
    delay(5);
  }
};

// کلاس برای مدیریت دکمه‌ها با دیبانس
class DebouncedButton {
private:
  uint8_t pin;             // شماره پین دکمه
  bool state;              // وضعیت فعلی دکمه
  bool lastState;          // وضعیت قبلی دکمه
  uint64_t lastTime;       // زمان آخرین تغییر وضعیت
  uint16_t debounceDelay;  // تاخیر دیبانس (به میلی‌ثانیه)

public:
  // سازنده: تنظیم مقادیر اولیه
  DebouncedButton(uint8_t btnPin, uint16_t delay = 50)
    : pin(btnPin), state(LOW), lastState(LOW), lastTime(0), debounceDelay(delay) {
    pinMode(pin, INPUT_PULLDOWN);  // تنظیم پین به‌عنوان ورودی
  }

  // متد برای بررسی وضعیت دکمه و اعمال دیبانس
  bool check() {
    bool reading = digitalRead(pin);  // خواندن وضعیت فعلی دکمه

    // بررسی تغییر وضعیت برای ریست تایمر دیبانس
    if (reading != lastState) {
      lastTime = millis();
    }

    // اعمال دیبانس بر اساس زمان
    if ((millis() - lastTime) > debounceDelay) {
      if (reading != state) {
        state = reading;  // به‌روزرسانی وضعیت دکمه

        // اگر دکمه فشار داده شده باشد، true بازگردانده شود
        if (state == HIGH) {
          lastState = reading;  // به‌روزرسانی وضعیت قبلی
          return true;
        }
      }
    }

    lastState = reading;  // به‌روزرسانی وضعیت قبلی
    return false;         // دکمه فشار داده نشده است
  }

  // متد برای دریافت وضعیت فعلی دکمه
  bool getState() const {
    return state;
  }

  // تنظیم تاخیر دیبانس
  void setDebounceDelay(uint16_t delay) {
    debounceDelay = delay;
  }
};

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
const String EXIT_KEYWORD = "exit";

LiquidCrystal lcd(LCD_ADDR, LCD_SDA, LCD_SCL);
DebouncedButton upBtn(UPBtn);
DebouncedButton downBtn(DOWNBtn);
DebouncedButton okBtn(OKBtn);

// end of lcd

// lcd menu
byte menuNumber = 0;
byte offset = 0;
byte arrow = 0;
bool isSubmenu = false;
byte currentMenu = menuNumber;
int sizeOfMenu = 0;
String menu[] = {
  "1)menu1~2)menu2~3)menu3~4)menu4~5)menu5",
  "1_1)menu1_1~1_2)menu1_2~1_3)menu1_3~1_4)exit",
  "",
  "3_1)menu3_1~3_2)menu3_2~3_3)menu3_3~3_4)exit",
  "",
  ""
};

// end of lcd menu

void setup() {
  lcd.begin();
  Serial.begin(115200);
  showMenu(menu, menuNumber, offset, arrow);
}

void loop() {
  handelButton();
}

  void calculateMenuItems(String menu[], byte menuNumber) {
    sizeOfMenu = 0;
    int i = 0;
    while (menu[menuNumber][i] != '\0') {
      if (menu[menuNumber][i++] == '~')
        sizeOfMenu++;
    }
    sizeOfMenu++;
  }

  String* splitMenuItems(String menu[], byte menuNumber, int sizeOfMenu) {
    // پوینتر به آرایه رشته‌ای دینامیک
    String* ar = new String[sizeOfMenu];
    int s = 0, e = 0;

    for (int i = 0; i < sizeOfMenu; i++) {
      e = menu[menuNumber].indexOf("~", s);
      ar[i] = menu[menuNumber].substring(s, e);
      s = e + 1;
      ar[i].trim();
    }

    return ar;
  }

  void showMenu(String menu[], byte menuNumber, byte offset, byte arrow) {

    calculateMenuItems(menu, menuNumber);

    // فراخوانی تابع splitMenuItems برای دریافت آیتم‌های منو
    String* ar = splitMenuItems(menu, menuNumber, sizeOfMenu);

    lcd.clear();
    for (int i = offset; i < offset + lcdRows && i < sizeOfMenu; i++) {  // بررسی محدوده
      if (i == offset + arrow) {
        lcd.setCursor(0, i - offset);
        lcd.write(">");
      }
      lcd.setCursor(2, i - offset);
      lcd.write(ar[i]);
    }

    // آزادسازی حافظه دینامیک
    delete[] ar;
  }


  void moveArrow(int x, int menuLenght) {
    byte end = min(lcdRows, menuLenght);
    if (x < 0) {
      if (arrow > 0)
        arrow += x;
      else if (offset > 0)
        offset += x;
    } else {
      if (arrow < end - 1)
        arrow += x;
      else if (offset < menuLenght - lcdRows)
        offset += x;
    }
  }

  int getSelectedMenu(byte offset, byte arrow) {
    return offset + arrow + 1;
  }

  void handelButton() {
    if (downBtn.check()) {
      moveArrow(1, sizeOfMenu);
      showMenu(menu, currentMenu, offset, arrow);
    }

    if (upBtn.check()) {
      moveArrow(-1, sizeOfMenu);
      showMenu(menu, currentMenu, offset, arrow);
    }

    if (okBtn.check()) {
      int selected = getSelectedMenu(offset, arrow);

      handleMenuSelection(selected);
    }
  }

  void handleMenuSelection(int selected) {

    // تخصیص حافظه دینامیک
    String* ar = splitMenuItems(menu, currentMenu, sizeOfMenu);

    if (ar != nullptr) {  // بررسی صحت تخصیص حافظه
      String selectedItem = ar[selected - 1];

      // آزادسازی حافظه پس از استفاده
      delete[] ar;

      if (!isSubmenu) {
        if (menu[selected].length() > 0) {
          selectMenu(selected);
          showMenu(menu, currentMenu, offset, arrow);
          isSubmenu = true;
        } else {
          actionMenu(currentMenu, selected);
        }
      } else {
        if (selectedItem.indexOf("exit") != -1) {
          exit();
          showMenu(menu, currentMenu, offset, arrow);
          isSubmenu = false;
        } else {
          actionMenu(currentMenu, selected);
        }
      }
    } else {
      // اگر تخصیص حافظه ناموفق باشد
      Serial.println("Error: Memory allocation failed");
    }
  }

  void exit() {
    // تنظیم مقادیر به حالت پیش‌فرض برای بازگشت به منوی اصلی
    menuNumber = 0;   // شماره منو به حالت پیش‌فرض
    arrow = 0;        // موقعیت نشانگر به حالت پیش‌فرض
    offset = 0;       // جبران موقعیت منو
    currentMenu = 0;  // بازگشت به منوی اصلی
  }

  void actionMenu(int currentMenu, int selectedMenu) {
    // چاپ شماره منو و زیرمنو برای اشکال‌زدایی
    Serial.printf("Current Menu: %d, Selected Menu: %d\n", currentMenu, selectedMenu);
  }

  void selectMenu(int selectedMenu) {
    // تغییر به منوی انتخاب‌شده و بازنشانی تنظیمات نشانگر
    currentMenu = selectedMenu;  // تغییر منوی فعلی به منوی انتخاب‌شده
    menuNumber = 0;              // بازنشانی شماره منو
    offset = 0;                  // بازنشانی جبران موقعیت
    arrow = 0;                   // بازنشانی نشانگر
  }
