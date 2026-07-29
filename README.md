# STM32F411 WS2812B 제어

## 프로젝트 개요

STM32F411의 TIM2 PWM과 DMA를 이용하여 WS2812B LED 4개를 제어한다.

LED 데이터 전송이 끝나면 PB10을 Low 상태로 유지하고, TIM4 One-Shot 타이머를 이용해 WS2812B의 Reset/Latch 시간을 확보한다.

## 사용 기능

* TIM2 Channel 3 PWM 출력
* PB10 Alternate Function
* DMA1 Stream1 Channel 3
* TIM4 One-Shot 타이머
* WS2812B LED 4개 제어
* LED 데이터 순서: G-R-B

## 데이터 구성

WS2812B 한 개는 24비트의 색상 데이터를 사용한다.

```text
Green 8비트 + Red 8비트 + Blue 8비트 = 24비트
```

LED 4개를 제어하므로 DMA 전송 데이터는 총 96개이다.

```text
24 × 4 = 96
```

`make_RGB()` 함수는 호출할 때마다 한 LED의 24비트 데이터를 버퍼에 연속 저장한다.

LED 4개의 데이터를 만들기 전에는 버퍼 인덱스를 반드시 0으로 초기화해야 한다.

```text
1번째 호출: buff[0]  ~ buff[23]
2번째 호출: buff[24] ~ buff[47]
3번째 호출: buff[48] ~ buff[71]
4번째 호출: buff[72] ~ buff[95]
```

## 전체 동작 순서

```text
PB10 Low 출력
→ TIM4 Reset 시간 대기
→ LED 4개 데이터 생성
→ TIM2 PWM 설정
→ DMA 설정 및 시작
→ TIM2 시작
→ PWM 데이터 96개 전송
→ DMA 완료 인터럽트
→ TIM2 정지
→ PB10 Low 출력
→ 다시 Reset 시간 대기
```

## TIM4 마이크로초 설정

TIM4 입력 클럭이 96MHz일 때 Prescaler를 95로 설정하면 타이머 카운트 주기가 1µs가 된다.

```text
96MHz ÷ 96 = 1MHz
1카운트 = 1µs
```

따라서 원하는 지연시간이 50µs이면 ARR은 49로 설정한다.

```text
50카운트 × 1µs = 50µs
```

마이크로초 지연을 사용할 때는 함수 인자를 밀리초 계산식과 혼용하지 않도록 주의한다.

## TIM4 인터럽트 주의사항

TIM4 설정값을 적용하기 위해 EGR의 UG 비트를 설정하면 UIF 플래그가 발생한다.

UIF를 제거하지 않은 상태에서 인터럽트를 활성화하면 실제 타이머 만료 전에 인터럽트가 발생할 수 있다.

올바른 설정 순서는 다음과 같다.

```text
타이머 정지
→ PSC 및 ARR 설정
→ UG 발생
→ UIF 제거
→ NVIC Pending 제거
→ 인터럽트 활성화
→ 타이머 시작
```

## TIM2와 DMA 시작 순서

TIM2를 DMA보다 먼저 시작하면 DMA 설정이 끝날 때까지 초기 CCR 값이 출력될 수 있다.

이로 인해 정상 PWM 데이터 앞에 불필요한 High 또는 Low 구간이 생길 수 있다.

올바른 시작 순서는 다음과 같다.

```text
RGB 버퍼 생성
→ TIM2 설정
→ DMA 설정 및 활성화
→ TIM2 시작
```

TIM2 출력 극성은 반전하지 않고 Active High로 사용한다.

## DMA 완료 처리

DMA 완료 인터럽트가 발생하면 96개의 PWM 데이터 전송이 끝난 상태이다.

DMA 완료 시 다음 처리를 수행한다.

* TIM2의 DMA 요청 비활성화
* TIM2 정지
* CCR3 초기화
* PB10을 GPIO 출력 Low로 변경
* DMA 동작 상태 플래그 초기화

PB10을 Low 상태로 변경한 뒤 TIM4 지연을 시작하여 WS2812B Reset/Latch 시간을 만든다.

## 해결한 문제

### 설정 시간보다 짧은 TIM4 지연

UG로 발생한 UIF가 실제 타이머 만료 인터럽트처럼 처리되어 지연시간이 짧게 측정되었다.

UIF와 NVIC Pending을 제거한 후 인터럽트를 활성화하여 해결하였다.


## 핵심 정리

* LED 4개 데이터 크기: 96개
* WS2812B 데이터 순서: G-R-B
* LED 데이터 생성 전 버퍼 인덱스 초기화
* TIM4의 한 카운트: 1µs
* 50µs 설정: PSC 95, ARR 49
* UG 발생 후 UIF 제거
* DMA 설정 후 TIM2 시작
* DMA 완료 후 TIM2 정지 및 PB10 Low 출력

