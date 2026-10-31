// panel_max.h
const char HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html><head>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>BuddyMax</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#080808;color:#00ffcc;font-family:'Courier New',monospace;padding:14px;max-width:540px;margin:auto}
h1{text-align:center;font-size:1.6em;letter-spacing:5px;color:#00ffcc;text-shadow:0 0 14px #00ffcc;margin-bottom:2px}
.sub{text-align:center;font-size:.7em;color:#444;margin-bottom:4px;letter-spacing:2px}
.ptabs{display:flex;gap:8px;margin-bottom:16px;justify-content:center}
.ptab{flex:1;max-width:160px;padding:10px;border-radius:10px;border:1px solid #00ffcc33;background:#0a0a0a;
  color:#00ffcc;font-family:inherit;font-size:.8em;cursor:pointer;text-align:center;transition:all .2s;letter-spacing:2px}
.ptab:hover{background:#00ffcc22}
.ptab.sel{background:#00ffcc;color:#000;box-shadow:0 0 14px #00ffcc}
.card{background:#0f0f0f;border:1px solid #1a1a1a;border-radius:12px;padding:14px;margin-bottom:12px}
.card h2{font-size:.75em;letter-spacing:3px;color:#444;margin-bottom:10px;border-bottom:1px solid #1a1a1a;padding-bottom:6px}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(96px,1fr));gap:6px}
button{display:block;width:100%;background:#080808;color:#00ffcc;border:1px solid #00ffcc22;border-radius:8px;
  padding:9px 4px;font-family:inherit;font-size:.73em;cursor:pointer;transition:all .18s;letter-spacing:1px}
button:hover{background:#00ffcc;color:#000;box-shadow:0 0 10px #00ffcc88}
button.act{background:#00ffcc18;border-color:#00ffcc;box-shadow:0 0 6px #00ffcc55}
.pm{border-color:#ff00cc22;color:#ff00cc}
.pm:hover{background:#ff00cc;color:#000;box-shadow:0 0 10px #ff00cc88}
.pm.act{background:#ff00cc18;border-color:#ff00cc;box-shadow:0 0 6px #ff00cc55}
.row{display:flex;align-items:center;gap:8px;margin-bottom:9px}
label{font-size:.7em;color:#444;min-width:76px;letter-spacing:1px}
input[type=range]{flex:1;accent-color:#00ffcc;height:4px}
input[type=color]{width:44px;height:32px;border:1px solid #222;border-radius:6px;background:none;cursor:pointer;padding:2px}
input[type=text]{flex:1;background:#080808;border:1px solid #00ffcc33;border-radius:6px;color:#00ffcc;
  font-family:inherit;font-size:.75em;padding:7px 8px;outline:none}
input[type=text]:focus{border-color:#00ffcc88}
.val{font-size:.7em;color:#00ffcc66;min-width:30px;text-align:right}
.bubble{background:#00ffcc0a;border:1px solid #00ffcc22;border-radius:8px;padding:8px 10px;
  font-size:.75em;color:#00ffcc88;min-height:30px;margin-bottom:8px}
.statgrid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:6px}
.stat{background:#080808;border:1px solid #1a1a1a;border-radius:8px;padding:8px 10px}
.stat .lbl{font-size:.58em;color:#444;letter-spacing:2px}
.stat .v{font-size:.95em;color:#00ffcc;margin-top:3px}
.colorrow{display:flex;flex-wrap:wrap;gap:5px;margin-top:6px}
.cc{width:26px;height:26px;border-radius:5px;border:1px solid #222;cursor:pointer;transition:transform .15s;display:flex;align-items:center;justify-content:center;font-size:.6em}
.cc:hover{transform:scale(1.25)}
.sep{height:1px;background:#1a1a1a;margin:8px 0}
</style></head><body>

<h1>&#x25C6; BUDDY MAX &#x25C6;</h1>
<div class='sub' id='subTitle'>COMBINED EDITION &nbsp;|&nbsp; WIFI CONTROL</div>

<div class='ptabs'>
  <div class='ptab sel' id='p0' onclick='setPersonality(0)'>&#x1F600; BUDDY</div>
  <div class='ptab' id='p1' onclick='setPersonality(1)'>&#x1F916; ROBOT</div>
</div>

<div class='card'>
  <h2>&#x1F4AC; SPEECH BUBBLE</h2>
  <div class='bubble' id='lastMsg'>No message yet...</div>
  <div class='row'>
    <input type='text' id='msgIn' placeholder='Type anything... (max 18)' maxlength='18'>
    <button style='width:64px' onclick='sendMsg()'>SEND</button>
  </div>
  <div class='grid' id='phraseGrid'></div>
</div>

<div class='card'>
  <h2>&#x25B6; EXPRESSIONS</h2>
  <div class='grid'>
    <button onclick='expr(0)'>&#x25CB; IDLE</button>
    <button onclick='expr(1)'>&#x1F604; HAPPY</button>
    <button onclick='expr(2)'>&#x1F621; ANGRY</button>
    <button onclick='expr(3)'>&#x1F622; SAD</button>
    <button onclick='expr(4)'>&#x1F632; SURPRISED</button>
    <button onclick='expr(5)'>&#x1F914; CURIOUS</button>
    <button onclick='expr(6)'>&#x1F609; WINK</button>
    <button onclick='expr(7)'>&#x2764; LOVE</button>
    <button onclick='expr(8)'>&#x1F635; DIZZY</button>
    <button onclick='expr(9)'>&#x1F634; SLEEPY</button>
    <button onclick='expr(10)'>&#x2753; CONFUSED</button>
    <button onclick='expr(11)'>&#x26A1; EXCITED</button>
    <button onclick='cmd("blink")'>&#x1F441; BLINK</button>
    <button onclick='cmd("scanline")'>&#x1F4FA; SCANLINE</button>
  </div>
</div>

<div class='card'>
  <h2>&#x2728; MODES</h2>
  <div class='grid'>
    <button class='pm' id='m0' onclick='setMode(0)'>&#x23F9; IDLE</button>
    <button class='pm' id='m1' onclick='setMode(1)'>&#x1F500; RANDOM</button>
    <button class='pm' id='m2' onclick='setMode(2)'>&#x1F389; PARTY</button>
    <button class='pm' id='m3' onclick='setMode(3)'>&#x1F300; HYPNO</button>
    <button class='pm' id='m4' onclick='setMode(4)'>&#x2764; HEARTBEAT</button>
    <button class='pm' id='m5' onclick='setMode(5)'>&#x1F57A; DISCO</button>
    <button class='pm' id='m6' onclick='setMode(6)'>&#x26A0; ALERT</button>
    <button class='pm' id='m7' onclick='setMode(7)'>&#x1F308; RAINBOW</button>
    <button class='pm' id='m8' onclick='setMode(8)'>&#x1F4AC; TALK</button>
  </div>
  <div class='sep'></div>
  <div class='row'><label>AUTO SPEED</label>
    <input type='range' min='500' max='10000' value='4000' id='autoInt' oninput='cmd("autoInt="+this.value);document.getElementById("autoIntV").innerText=(this.value/1000).toFixed(1)+"s"'>
    <span class='val' id='autoIntV'>4.0s</span>
  </div>
  <div class='row'><label>BLINK SPEED</label>
    <input type='range' min='1000' max='10000' value='4000' id='blinkInt' oninput='cmd("blinkInt="+this.value);document.getElementById("blinkIntV").innerText=(this.value/1000).toFixed(1)+"s"'>
    <span class='val' id='blinkIntV'>4.0s</span>
  </div>
</div>

<div class='card'>
  <h2>&#x1F441; EYE SHAPE</h2>
  <div class='row'><label>EYE COLOR</label>
    <input type='color' id='ecol' value='#00ffff' oninput='sendEyeColor(this.value)'>
    <span style='font-size:.68em;color:#444;margin-left:4px'>eye fill</span>
  </div>
  <div class='row'><label>BG COLOR</label>
    <input type='color' id='bgcol' value='#000000' oninput='sendBgColor(this.value)'>
    <span style='font-size:.68em;color:#444;margin-left:4px'>screen bg</span>
  </div>
  <div class='row'><label>WIDTH</label>
    <input type='range' min='8' max='50' value='28' id='eW' oninput='ep("eyeW",this.value,"eWv")'>
    <span class='val' id='eWv'>28</span>
  </div>
  <div class='row'><label>HEIGHT</label>
    <input type='range' min='6' max='58' value='38' id='eH' oninput='ep("eyeH",this.value,"eHv")'>
    <span class='val' id='eHv'>38</span>
  </div>
  <div class='row'><label>ROUNDNESS</label>
    <input type='range' min='0' max='20' value='7' id='eR' oninput='ep("eyeR",this.value,"eRv")'>
    <span class='val' id='eRv'>7</span>
  </div>
  <div class='row'><label>GAP</label>
    <input type='range' min='2' max='32' value='10' id='eG' oninput='ep("eyeG",this.value,"eGv")'>
    <span class='val' id='eGv'>10</span>
  </div>
  <div class='row'><label>POS X</label>
    <input type='range' min='-20' max='20' value='0' id='eOX' oninput='ep("eyeOX",this.value,"eOXv")'>
    <span class='val' id='eOXv'>0</span>
  </div>
  <div class='row'><label>POS Y</label>
    <input type='range' min='-20' max='20' value='0' id='eOY' oninput='ep("eyeOY",this.value,"eOYv")'>
    <span class='val' id='eOYv'>0</span>
  </div>
</div>

<div class='card'>
  <h2>&#x1F4A1; RGB LIGHT</h2>
  <div class='row'><label>COLOR</label>
    <input type='color' id='cpick' value='#00b4b4' oninput='sendColor(this.value)'>
  </div>
  <div class='row'><label>BRIGHTNESS</label>
    <input type='range' min='0' max='100' value='100' id='bri' oninput='cmd("bri="+this.value);document.getElementById("briv").innerText=this.value+"%"'>
    <span class='val' id='briv'>100%</span>
  </div>
  <div class='row'><label>RED</label>
    <input type='range' min='0' max='255' value='0' id='rr' oninput='sendRGB()'>
    <span class='val' id='rrv'>0</span>
  </div>
  <div class='row'><label>GREEN</label>
    <input type='range' min='0' max='255' value='180' id='rg' oninput='sendRGB()'>
    <span class='val' id='rgv'>180</span>
  </div>
  <div class='row'><label>BLUE</label>
    <input type='range' min='0' max='255' value='180' id='rb' oninput='sendRGB()'>
    <span class='val' id='rbv'>180</span>
  </div>
  <div class='colorrow'>
    <div class='cc' style='background:#f00' onclick='setRGBQ(255,0,0)'></div>
    <div class='cc' style='background:#0f0' onclick='setRGBQ(0,255,0)'></div>
    <div class='cc' style='background:#00f' onclick='setRGBQ(0,0,255)'></div>
    <div class='cc' style='background:#ff0' onclick='setRGBQ(255,255,0)'></div>
    <div class='cc' style='background:#0ff' onclick='setRGBQ(0,255,255)'></div>
    <div class='cc' style='background:#f0f' onclick='setRGBQ(255,0,255)'></div>
    <div class='cc' style='background:#fff' onclick='setRGBQ(255,255,255)'></div>
    <div class='cc' style='background:#ff8800' onclick='setRGBQ(255,136,0)'></div>
    <div class='cc' style='background:#8800ff' onclick='setRGBQ(136,0,255)'></div>
    <div class='cc' style='background:#ff4488' onclick='setRGBQ(255,68,136)'></div>
    <div class='cc' style='background:#00ff88' onclick='setRGBQ(0,255,136)'></div>
    <div class='cc' style='background:#111;border-color:#333;color:#555' onclick='setRGBQ(0,0,0)'>&#x2715;</div>
  </div>
</div>

<div class='card'>
  <h2>&#x1F50A; BUZZER</h2>
  <div class='row'><label>FREQUENCY</label>
    <input type='range' min='200' max='5000' value='3500' id='bfreq' oninput='document.getElementById("bfreqv").innerText=this.value+"Hz"'>
    <span class='val' id='bfreqv'>3500Hz</span>
  </div>
  <div class='row'><label>DURATION</label>
    <input type='range' min='20' max='1000' value='200' id='bdur' oninput='document.getElementById("bdurv").innerText=this.value+"ms"'>
    <span class='val' id='bdurv'>200ms</span>
  </div>
  <button style='margin-bottom:8px' onclick='cmd("beep="+document.getElementById("bfreq").value+"&bdur="+document.getElementById("bdur").value)'>&#x25B6; PLAY TONE</button>
  <div class='grid'>
    <button onclick='cmd("melody=0")'>&#x1F3B5; JINGLE</button>
    <button onclick='cmd("melody=1")'>&#x1F3AE; MARIO</button>
    <button onclick='cmd("melody=2")'>&#x1F614; SAD</button>
    <button onclick='cmd("melody=3")'>&#x26A0; ALERT</button>
    <button onclick='cmd("melody=4")'>&#x1F600; HAPPY</button>
    <button onclick='cmd("melody=5")'>&#x1F916; R2D2</button>
    <button onclick='cmd("melody=6")'>&#x1F4AA; IMPERIAL</button>
    <button onclick='cmd("buzz_toggle")' id='btog'>&#x1F507; MUTE</button>
  </div>
</div>

<div class='card'>
  <h2>&#x2699; ROBOT NAME</h2>
  <div class='row'>
    <input type='text' id='nameIn' placeholder='Enter name...' maxlength='23'>
    <button style='width:60px' onclick='sendName()'>SET</button>
  </div>
</div>

<div class='card'>
  <h2>&#x1F4CA; SYSTEM STATS</h2>
  <div class='statgrid'>
    <div class='stat'><div class='lbl'>UPTIME</div><div class='v' id='uptime'>--</div></div>
    <div class='stat'><div class='lbl'>FREE HEAP</div><div class='v' id='heap'>--</div></div>
    <div class='stat'><div class='lbl'>CLIENTS</div><div class='v' id='clients'>--</div></div>
    <div class='stat'><div class='lbl'>EXPRESSIONS</div><div class='v' id='exprct'>--</div></div>
    <div class='stat'><div class='lbl'>MESSAGES</div><div class='v' id='msgct'>--</div></div>
    <div class='stat'><div class='lbl'>BUZZER</div><div class='v' id='buzzst'>--</div></div>
    <div class='stat'><div class='lbl'>PERSONALITY</div><div class='v' id='persst'>--</div></div>
    <div class='stat'><div class='lbl'>MODE</div><div class='v' id='modest'>--</div></div>
    <div class='stat'><div class='lbl'>IP</div><div class='v'>4.1</div></div>
  </div>
</div>

<script>
const buddyPhrases=['Hello there!','I am BUDDY','Beep boop!','How are you?','I feel great!','Whoa!','Zzz...','I like you!','Error 404','Lets party!','Ouch!','Hehe ;)','So excited!','I am dizzy','Miss you!'];
const robotPhrases=['ONLINE','SCANNING...','BEEP BOOP','HELLO HUMAN','PROCESSING','LOW BATTERY','REBOOTING','ALERT!','ALL CLEAR','UNIT OK','CHARGING','STANDBY','INITIALIZING','CALIBRATING','READY'];
const modeNames=['IDLE','RANDOM','PARTY','HYPNO','HEARTBEAT','DISCO','ALERT','RAINBOW','TALK'];
let curPersonality=0;

function cmd(c){fetch('/cmd?'+c);}
function expr(e){cmd('expr='+e);}
function setMode(m){
  cmd('mode='+m);
  document.querySelectorAll('.pm').forEach((b,i)=>b.classList.toggle('act',i==m));
}
function setPersonality(p){
  curPersonality=p;
  cmd('personality='+p);
  document.getElementById('p0').classList.toggle('sel',p==0);
  document.getElementById('p1').classList.toggle('sel',p==1);
  buildPhrases();
  // Update eye sliders to match preset
  if(p==0){
    setSlider('eW',28);setSlider('eH',38);setSlider('eR',7);setSlider('eG',10);
  } else {
    setSlider('eW',30);setSlider('eH',30);setSlider('eR',4);setSlider('eG',8);
  }
}
function setSlider(id,val){
  document.getElementById(id).value=val;
  document.getElementById(id+'v').innerText=val;
}
function buildPhrases(){
  const ph=curPersonality==0?buddyPhrases:robotPhrases;
  const g=document.getElementById('phraseGrid');
  g.innerHTML='';
  ph.forEach((p,i)=>{
    const b=document.createElement('button');
    b.textContent=p.length>12?p.substring(0,11)+'...':p;
    b.title=p;
    b.onclick=()=>{cmd('phrase='+i);document.getElementById('lastMsg').innerText=p;};
    g.appendChild(b);
  });
}
function sendMsg(){
  const t=document.getElementById('msgIn').value.trim();
  if(!t)return;
  cmd('msg='+encodeURIComponent(t));
  document.getElementById('lastMsg').innerText=t;
  document.getElementById('msgIn').value='';
}
function sendColor(hex){
  const r=parseInt(hex.slice(1,3),16),g=parseInt(hex.slice(3,5),16),b=parseInt(hex.slice(5,7),16);
  cmd('rgb='+r+','+g+','+b);
  document.getElementById('rr').value=r;document.getElementById('rrv').innerText=r;
  document.getElementById('rg').value=g;document.getElementById('rgv').innerText=g;
  document.getElementById('rb').value=b;document.getElementById('rbv').innerText=b;
}
function sendRGB(){
  const r=document.getElementById('rr').value,g=document.getElementById('rg').value,b=document.getElementById('rb').value;
  document.getElementById('rrv').innerText=r;document.getElementById('rgv').innerText=g;document.getElementById('rbv').innerText=b;
  cmd('rgb='+r+','+g+','+b);
}
function setRGBQ(r,g,b){
  cmd('rgb='+r+','+g+','+b);
  document.getElementById('rr').value=r;document.getElementById('rrv').innerText=r;
  document.getElementById('rg').value=g;document.getElementById('rgv').innerText=g;
  document.getElementById('rb').value=b;document.getElementById('rbv').innerText=b;
}
function sendEyeColor(hex){
  const r=parseInt(hex.slice(1,3),16),g=parseInt(hex.slice(3,5),16),b=parseInt(hex.slice(5,7),16);
  cmd('eyecol='+r+','+g+','+b);
}
function sendBgColor(hex){
  const r=parseInt(hex.slice(1,3),16),g=parseInt(hex.slice(3,5),16),b=parseInt(hex.slice(5,7),16);
  cmd('bgcol='+r+','+g+','+b);
}
function ep(p,v,vid){cmd(p+'='+v);document.getElementById(vid).innerText=v;}
function sendName(){
  const n=document.getElementById('nameIn').value.trim();
  if(!n)return;
  cmd('name='+encodeURIComponent(n));
}
buildPhrases();
setInterval(()=>{
  fetch('/status').then(r=>r.json()).then(d=>{
    document.getElementById('uptime').innerText=d.uptime;
    document.getElementById('heap').innerText=d.heap+'KB';
    document.getElementById('clients').innerText=d.clients;
    document.getElementById('exprct').innerText=d.expressions;
    document.getElementById('msgct').innerText=d.messages;
    document.getElementById('buzzst').innerText=d.buzzer?'ON':'MUTED';
    document.getElementById('persst').innerText=d.personality==0?'BUDDY':'ROBOT';
    document.getElementById('modest').innerText=modeNames[d.mode]||'--';
  }).catch(()=>{});
},2500);
</script>
</body></html>
)rawhtml";
