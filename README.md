TTK4145
=======


Bugs:
  * Hvis en heis er i samme etasje som knappen trykkes i, så forsvinner ordren aldri fra ordrelisten. Tror det er fordi FSM sender FSMOrderCompleteEvent bare når state faktisk forandrer seg.

Todo:
  * VIKTIG Lysene på eksterne knapper må implementeres
  * Det nye ordresystemet fungerer bra, men evt. ha med timeout?
  * Muligens nye navn på Logic, ConnectionController og Sender?
  * Kommentere koden litt bedre
  * Figur(er)
