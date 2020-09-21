# TomoTableClient

Ze względu na dużą liczbę urządzeń USB, należy powiększyć bufory systemowe, dodając
```
 coherent_pool=4M
```
do pliku `cmdline.txt` w `/boot`.
