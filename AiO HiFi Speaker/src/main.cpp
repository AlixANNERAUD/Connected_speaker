#include "main.hpp"

void setup()
{
  esp_bt_controller_config_t Bluetooth_Configuration = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  if (esp_bt_controller_init(&Bluetooth_Configuration) != ESP_OK)
  {
    ESP_LOGE(BT_AV_TAG, "%s initialize controller failed\n", __func__);
    return;
  }
  if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK)
  {
    ESP_LOGE(BT_AV_TAG, "%s enable controller failed\n", __func__);
    return;
  }
  if (esp_bluedroid_init() != ESP_OK)
  {
    ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed\n", __func__);
    return;
  }

  if (esp_bluedroid_enable() != ESP_OK)
  {
    ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed\n", __func__);
    return;
  }

      renderer_init(renderer_config);

    /* create application task */
    bt_app_task_start_up();

    /* Bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);
  // put your setup code here, to run once:
}

void loop()
{
  vTaskDelete(NULL);
  // put your main code here, to run repeatedly:
}