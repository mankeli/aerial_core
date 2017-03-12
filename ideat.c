-joo generoi hiressi missä on 0-F slidet väreissä, sitten rutiini joka käy hiressiruudun läpi ja korvaa näitä värejä.
 ts. kopsaa yhdestä paikasta toiseen. esim 400->800 niin ei tarvi muuttaa laser.prg latausta

 
- alkuwriteri
- pallokallo
- zoomaustunneli
- plasmatunneli kahdella kuvalla
- 7row
- petscii-twister
- writer + petscii-silmä
- sprite-hires-kuva+väripyöritys
- rasterzoomer
- timantti
- logopartti


DONE:
- alkuwriteri
- pallokallo
- zoomaustunneli
- plasmatunneli kahdella kuvalla

EFFUJA:
- sprite-hires-kuva+väripyöritys
- rasterzoomer
- writer + petscii-silmä (11,7 positio)

EHKÄ:
- 7row
- petscii-twister
- timantti
- logopartti



- pallon ja kallon perusväripaletti
  - fiksaa myös valonlähde, ota aina vaan seuraavat värit paletista (1st try)
- värien cleraus ku ladataan uus kuva


PARTY TODO:::

- joo, tee .hasm:t molemmille logoille! DONE


final todo:
x lft nimi creduihin
- pidempi sinus rotoon
- twisteriin lisää nopeutta tms. siinä on joku bugi?
x musan looppaus. lataa vaikka 9000 alueelle ja kopsaa alas jos musa ei vielä soi?
- lisää spritejä ekoihin partteihin, tai muuten vaan lyhyemmät lekottelut
x extend petscii hajoaa myöhemmillä looppauskerroilla wtf?? ilmeisesti näyttömuisti vaihdetaan väärin
- rotozoomeri bugaa kun se on tasakohdassa. wat?

- rotozoomer

CHANGELOG partysta:
 - musiikki ei enää bugaa loopatessa
 - ruudun yläreuna ei bugaa laaserista siirryttäessä logoon
   (kusee toisella tavalla kun näkyy petsciitä)
 - logo ei välillä häviä ruudusta ja bugaudu
 - laser alkaa nopeemmin
 - silmäräjähdys ei pitäisi bugata enää missään tapauksessa, plus räjähdys tulee
   hivenen aiemmin että animaation pitäisi aina mennä loppuun asti
 - toisen writerin tekstejä muutettu jonkin verran
 - silmien animaatio lähtee käyntiin (sulavammin)
 - silmät asettuvat samalle paikalle mistä räjähdys lähtee
 - twisterin päivitysbugi korjattu
