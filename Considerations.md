# 3. MQTT Control Packets

## 3.1. CONNECT

The first packet sent from the Client to the Server **MUST** be a CONNECT packet. If a Client send a second CONNECT packet, the Server **MUST** process this packet as a protocol violations and disconnect the Client.

### 3.1.2. Variable header

- Protocol Name (byte 1 - 6)
- Protocol Level (byte 7)
- Connect Flags (byte 8)
- Keep Alive (byte 9 - 10)

#### 3.1.2.1. Protocol Name

If the protocol name is incorrect the Server **MUST** disconnect the Client.

#### 3.1.2.2. Protocol Level

If the protocol level(4) is not supported by the Server, then it **MUST** disconnect the Client.

#### 3.1.2.3. Connect Flags

If the reserved flag is not set to zero, then the Server **MUST** disconnect the Client.

**Check the meaning of the other bits**

### 3.1.3. Payload

The presence of the payload is determined by the flags in the [Variable header](#312-variable-header). These fields, if present, **MUST** appear in the following order:
1. Client Identifier (UTF-8)
2. Will Topic (UTF-8)
3. Will Message
4. User Name (UTF-8)
5. Password

### 3.1.3. Response

- If the server does not receive a CONNECT Packet within a reasonable amount of time after the connection, it **SHOULD** close the connection.
- If the ClientId represents a Client already connected, the Server **SHOULD** disconnect the existing Client.

## 3.3. PUBLISH

## 3.14. DISCONNECT