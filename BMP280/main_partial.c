
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "BMP280_simple.h"
/* USER CODE END Includes */


/* USER CODE BEGIN 2 */

  printf("\r\nChecking for BMP280\r\n");
  BMP280_check();
  printf("\r\nConfigure BMP280\r\n");
  BMP280_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  BMP280_get_temperature();
	  BMP280_get_pressure();
	  HAL_Delay(1000);
	  /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
