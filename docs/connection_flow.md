# Connection Flow Diagrams

## System Architecture Overview

```mermaid
graph TB
    subgraph "Power System"
        PS12V[12V 5A Power Supply]
        PS5V[5V 2A Power Supply]
    end

    subgraph "Control System"
        ESP32[ESP32 Development Board]
        RADIO[nRF24L01+ Radio Module]
        LEVEL[Logic Level Shifter]
    end

    subgraph "Display System"
        LED1[LED Strip Digit 1]
        LED2[LED Strip Digit 2]
    end

    PS12V --> LED1
    PS12V --> LED2
    PS5V --> ESP32

    ESP32 -->|SPI| RADIO
    ESP32 -->|GPIO| LEVEL
    LEVEL --> LED1
    LEVEL --> LED2

    CONTROLLER[Controller Module] -->|868 MHz Radio| RADIO
```

## Detailed Pin Connections

### Radio Module Connections

```mermaid
graph LR
    subgraph "ESP32"
        ESP32_VCC[3V3 Pin]
        ESP32_GND[GND Pin]
        ESP22[GPIO22 - CE]
        ESP21[GPIO21 - CSN]
        ESP18[GPIO18 - SCK]
        ESP23[GPIO23 - MOSI]
        ESP19[GPIO19 - MISO]
    end

    subgraph "nRF24L01+ Module"
        RADIO_VCC[VCC]
        RADIO_GND[GND]
        RADIO_CE[CE]
        RADIO_CSN[CSN]
        RADIO_SCK[SCK]
        RADIO_MOSI[MOSI]
        RADIO_MISO[MISO]
        RADIO_IRQ[IRQ]
    end

    ESP32_VCC --> RADIO_VCC
    ESP32_GND --> RADIO_GND
    ESP22 --> RADIO_CE
    ESP21 --> RADIO_CSN
    ESP18 --> RADIO_SCK
    ESP23 --> RADIO_MOSI
    ESP19 --> RADIO_MISO
```

### LED Display Connections

```mermaid
graph TB
    subgraph "Power Distribution"
        PS12V[12V 5A Supply]
        CAP[1000µF Capacitor]
    end

    subgraph "Control"
        ESP32[ESP32]
        ESP32_GPIO4[GPIO4 - Power Control]
        ESP32_GPIO5[GPIO5 - Data]
        LEVEL[Logic Level Shifter 3.3V→5V]
    end

    subgraph "LED Strips"
        LED1[Digit 1 - 100cm]
        LED2[Digit 2 - 100cm]

        subgraph "LED1 Connections"
            LED1_VCC[+12V]
            LED1_GND[GND]
            LED1_DI[Data Input]
            LED1_BI[Backup Input]
        end

        subgraph "LED2 Connections"
            LED2_VCC[+12V]
            LED2_GND[GND]
            LED2_DI[Data Input]
            LED2_BI[Backup Input]
        end
    end

    PS12V --> CAP
    CAP --> LED1_VCC
    CAP --> LED2_VCC

    ESP32_GPIO4 -->|Power Control| LED1_VCC
    ESP32_GPIO4 -->|Power Control| LED2_VCC

    ESP32_GPIO5 --> LEVEL
    LEVEL --> LED1_DI
    LED1_DI --> LED1_BI

    LEVEL --> LED2_DI
    LED2_DI --> LED2_BI
```

## Power Flow Diagram

```mermaid
flowchart TD
    subgraph "AC Power"
        WALL[Wall Outlet 230V AC]
    end

    subgraph "Power Supplies"
        PS12V[12V 5A DC Supply]
        PS5V[5V 2A DC Supply]
    end

    subgraph "Power Regulation"
        ESP32_REG[ESP32 3.3V Regulator]
    end

    subgraph "Load Distribution"
        ESP32_LOAD[ESP32 Board]
        RADIO_LOAD[Radio Module 3.3V]
        LED_LOAD[LED Strips 12V]
    end

    WALL --> PS12V
    WALL --> PS5V

    PS12V --> LED_LOAD

    PS5V --> ESP32_LOAD
    ESP32_LOAD --> ESP32_REG
    ESP32_REG --> RADIO_LOAD
```

## Signal Flow for Data Reception

```mermaid
sequenceDiagram
    participant Controller as Controller Module
    participant Radio as nRF24L01+ Module
    participant ESP32 as ESP32 Processor
    participant Logic as Logic Level Shifter
    participant LED as LED Strips

    Controller->>Radio: 868 MHz Status Frame
    Radio->>ESP32: SPI Data (MISO)
    ESP32->>ESP32: Parse System State
    ESP32->>ESP32: Update Display Data
    ESP32->>Logic: GPIO5 Data Signal
    Logic->>LED: 5V WS2815 Data
    LED->>LED: Update Display
```

## Ground System Diagram

```mermaid
graph TB
    subgraph "Ground Points"
        GND_ESP32[ESP32 Ground]
        GND_RADIO[Radio Module Ground]
        GND_LEVEL[Logic Shifter Ground]
        GND_LED1[LED Strip 1 Ground]
        GND_LED2[LED Strip 2 Ground]
        GND_PS5V[5V Supply Ground]
        GND_PS12V[12V Supply Ground]
    end

    subgraph "Common Ground Bus"
        GND_BUS[Main Ground Bus]
    end

    GND_ESP32 --> GND_BUS
    GND_RADIO --> GND_BUS
    GND_LEVEL --> GND_BUS
    GND_LED1 --> GND_BUS
    GND_LED2 --> GND_BUS
    GND_PS5V --> GND_BUS
    GND_PS12V --> GND_BUS
```

## LED Strip Segment Layout

```mermaid
graph TB
    subgraph "7-Segment Display Layout"
        subgraph "Digit 1 (Tens)"
            A_TOP[Segment A - Top]
            B_TOP[Segment B - Top Right]
            C_TOP[Segment C - Bottom Right]
            D_TOP[Segment D - Bottom]
            E_TOP[Segment E - Bottom Left]
            F_TOP[Segment F - Top Left]
            G_TOP[Segment G - Middle]
            DP_TOP[Decimal Point]
        end

        subgraph "Digit 2 (Units)"
            A_BOTTOM[Segment A - Top]
            B_BOTTOM[Segment B - Top Right]
            C_BOTTOM[Segment C - Bottom Right]
            D_BOTTOM[Segment D - Bottom]
            E_BOTTOM[Segment E - Bottom Left]
            F_BOTTOM[Segment F - Top Left]
            G_BOTTOM[Segment G - Middle]
            DP_BOTTOM[Decimal Point]
        end
    end

    subgraph "LED Strip Layout"
        STRIP1[LED Strip 1 - 100cm]
        STRIP2[LED Strip 2 - 100cm]
    end

    STRIP1 --> A_TOP
    STRIP1 --> B_TOP
    STRIP1 --> C_TOP
    STRIP1 --> D_TOP
    STRIP1 --> E_TOP
    STRIP1 --> F_TOP
    STRIP1 --> G_TOP
    STRIP1 --> DP_TOP

    STRIP2 --> A_BOTTOM
    STRIP2 --> B_BOTTOM
    STRIP2 --> C_BOTTOM
    STRIP2 --> D_BOTTOM
    STRIP2 --> E_BOTTOM
    STRIP2 --> F_BOTTOM
    STRIP2 --> G_BOTTOM
    STRIP2 --> DP_BOTTOM
```

## Physical Layout建议

```mermaid
graph TB
    subgraph "Enclosure Layout (Top View)"
        subgraph "Left Side"
            ESP32_POS[ESP32 Board]
            RADIO_POS[Radio Module]
            LEVEL_POS[Logic Shifter]
        end

        subgraph "Right Side"
            POWER_POS[Power Supplies]
            CAP_POS[Capacitors]
        end

        subgraph "Bottom"
            LED1_POS[LED Digit 1]
            LED2_POS[LED Digit 2]
        end
    end

    ESP32_POS -->|Short Wires| RADIO_POS
    ESP32_POS --> LEVEL_POS
    POWER_POS --> CAP_POS
    CAP_POS --> LED1_POS
    CAP_POS --> LED2_POS
```