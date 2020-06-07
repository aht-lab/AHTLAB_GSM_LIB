# AHTLAB GSM Library
**Thư viện hỗ trợ phần cứng [ESP32 GSM](https://ahtlab.com/san-pham/kit-wifi-esp32-gsm-2g-3g/) được thiết kế bởi [AHTLAB.COM](https://ahtlab.com/)**
![](https://ahtlab.com/ahtlab/uploads/2019/11/esp32_sim_1-768x576.png)
## Các hàm hỗ trợ
 - Tự nhận baudrate, loại GSM (Hiện tại hỗ trợ SIM800, Tương lai SIM5300, UC15)
    ```c++
    // define
    AHT_GSM *gsmMaster = new AHT_GSM(&uart);
    AHT_GSM *gsm;
    GSM_TYPE gsmType;
    
    // void setup
    gsmType = gsmMaster->detectGSM(&uart);
    unsigned  long baudrate = gsmMaster->getBaudrate();
    free(gsmMaster);
    if(gsmType == SIM800)
    {
    	gsm = new  AHT_SIM800(&uart);
    	gsm->begin(baudrate);
    }
    ```
 - Gọi
   - Hàm gọi điện 1: Tính chính xác thời gian gọi
   ```c++
	char phone[] = "0946866793";
	bool canCall = gsm->setupCall();
	if(canCall)
	{
		Serial.println("canCall");
		// call(<phone>, <thời gian chờ nghe máy>(giây), <thời gian gọi>(giây))
		// return: <thời gian 2 bên nghe máy>
		// *Lưu ý: 
		// 			<thời gian chờ nghe máy> nên phù hợp để tránh tính trạng 
		//			tổng đài yêu cầu gửi lời nhắn thoại.
		//			Trong trường hợp đó dù cuộc gọi không thành công
		//			nhưng vẫn sẽ mất thời gian gọi điện.
		uint16_t timeCall = gsm->call(phone, 35, 5);
		Serial.print("time call: ");
		Serial.println(timeCall);
	}
   ```
   - Hàm gọi điện 2
   ```c++
    gsm->call("0946866793");
	unsigned  long now = millis();
	unsigned  long TIMEOUT = 15000;
	while(millis() - now < TIMEOUT)
	{
		int callActive = gsm->phoneActiveSTT();
		Serial.println("Call Active: " + String(callActive));
		delay(1000);
	}
	gsm->handup();
	```
 - SMS
   - Đếm số lượng SMS hiện tại
   - Đọc SMS với index chỉ định
   - Gửi SMS không dấu (Hàm **gửi sms có dấu** chưa được **AHTLAB** public)
   - Xóa tất cả các SMS
   ```c++
    uint8_t numSMS = gsm->numSMS(SMS_STT_ALL);
	gsm->readSMS(numSMS);
	gsm->deleteAllSMS();
    if(gsm->sendSMS("0946866793", "cafe5hsang@gmail.com - mita9497dev@gmail.com"))
	{
		Serial.println("Gui SMS thanh cong");
	}
	```
  - HTTP GET Request
    ```c++
	#define HOST            "5ea8480235f3720016608d00.mockapi.io"
	#define PATH            "/api/test"
	#define BODY_LEN 512
	char body[BODY_LEN ];
	
    bool gsmNet = gsm->attackGPRS("internet.wind", "", "");
    if(gsmNet ) 
    {
	    bool success = gsm->requestGet(HOST, PATH, 80, body, BODY_LEN);
	    if(success) 
	    {
		    Serial.print("body: ");
		    Serial.println(body);
	    }
    }
	```
  - HTTP POST Request
    ```c++
	#define HOST            "5ea8480235f3720016608d00.mockapi.io"
	#define PATH            "/api/test"
	#define BODY_LEN 512
	char body[BODY_LEN];

	char data[] = "ping=1&pong=1";
	
    bool gsmNet = gsm->attackGPRS("internet.wind", "", "");
    if(gsmNet ) 
    {
	    bool success = gsm->requestPost(HOST, PATH, 80, data, body, BODY_LEN);
	    if(success) 
	    {
		    Serial.print("body: ");
		    Serial.println(body);
	    }
    }
	```
  - Get IMEI
    ```c++
    char IMEI_GSM[20], IMEI_SIM[20];
    
	gsm->getIMEI(IMEI_GSM);
	Serial.print("IMEI GSM: ");
	Serial.println(IMEI_GSM);

	gsm->getIMEI(IMEI_SIM);
	Serial.print("IMEI SIM: ");
	Serial.println(IMEI_SIM);
	```
  - CELL ID
  - Get Location
	```c++
	char  LAT[10], LNG[10];
	char  CELL_ID[6], LAC[6];
	if(gsm->getLocation(LAT, LNG))
	{
		Serial.println("Get location success");
	}
	else
	{
		Serial.println("Fail to get location");
		gsm->getCellId(CELL_ID, LAC);
	}
	```

  - NTP Time
    ```c++
	uint16_t year, month, day, hour, minute, second, timezone;
	bool timeInit = gsm->setupNTP();
	if (timeInit)
	{
		gsm->getTimeNTP(&year, &month, &day, &hour, &minute, &second, &timezone);
		Serial.print("year: ");
		Serial.println(year);
		Serial.print("month: ");
		Serial.println(month);
		Serial.print("day: ");
		Serial.println(day);
		Serial.print("hour: ");
		Serial.println(hour);
		Serial.print("minute: ");
		Serial.println(minute);
		Serial.print("second: ");
		Serial.println(second);
		Serial.print("timezone: ");
		Serial.println(timezone);
	}
	```
