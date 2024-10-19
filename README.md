<img src=".github/remi.png" width="300">

# 🎈 remi

That is Remi. You can request or send things to Remi store. But Remi can be a little 
dumb. Don't assume that you'll receive what you've requested or that she will store 
what you've sent. (No, this is not a excuse to my poor implementation, I promise.).

To help Remi in her duty and don't have some headache in using her service, please, 
send the request following the correct structure.

## 🩴 Basic You-Remi Intercommunication Diagram

```
You                    Remi!
 |      ADD key val      |
 |---------------------->|
 |                       |
 | Good job my friend :) |
 |<----------------------|
 |                       |
 |                       |
```

## 🔭 Remi Protocol Specification

The Remi Protocol is very simple to Remi can understand. You understand?

```
+--------+---------+--------+---------+---------
| length | message | length | message | more... 
+--------+---------+--------+---------+---------
```

Each message must have two fields, the `length` (4 bytes) that is the message 
length  and the `message` (n bytes) it self. A client can send multiple messages 
to Remi. But be careful, some misspelled message can confuse Remi.