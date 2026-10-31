// panel_buddy.h — auto-included by Alive_Demo.ino
const char HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html><head>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Buddy Robot</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0a0a0a;color:#00ffcc;font-family:'Courier New',monospace;padding:14px;max-width:520px;margin:auto}
h1{text-align:center;font-size:1.5em;letter-spacing:4px;color:#00ffcc;text-shadow:0 0 12px #00ffcc;margin-bottom:2px}
.sub{text-align:center;font-size:.7em;color:#444;margin-bottom:16px;letter-spacing:2px}
.card{background:#111;border:1px solid #1c1c1c;border-radius:12px;padding:14px;margin-bottom:12px}
.card h2{font-size:.75em;letter-spacing:3px;color:#444;margin-bottom:10px;border-bottom:1px solid #1c1c1c;padding-bottom:6px}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(100px,1fr));gap:7px}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:7px}
btn,button{display:block;width:100%;background:#0a0a0a;color:#00ffcc;border:1px solid #00ffcc22;border-radius:8px;
  padding:9px 4px;font-family:inherit;font-size:.75em;cursor:pointer;transition:all .18s;letter-spacing:1px;text-align:center}
button:hover,btn:hover{background:#00ffcc;color:#000;box-shadow:0 0 10px #00ffcc88}
button.act{background:#00ffcc18;border-color:#00ffcc;box-shadow:0 0 6px #00ffcc44}
.pm{border-color:#ff00cc22;color:#ff00cc}
.pm:hover{background:#ff00cc;color:#000;box-shadow:0 0 10px #ff00cc88}
.pm.act{background:#ff00cc18;border-color:#ff00cc}
.row{display:flex;align-items:center;gap:8px;margin-bottom:9px}
label{font-size:.7em;color:#444;min-width:72px;letter-spacing:1px}
input[type=range]{flex:1;accent-color:#00ffcc;height:4px}
input[type=color]{width:44px;height:32px;border:1px solid #222;border-radius:6px;background:none;cursor:pointer;padding:2px}
input[type=text]{flex:1;background:#0a0a0a;border:1px solid #00ffcc33;border-radius:6px;color:#00ffcc;
  font-family:inherit;font-size:.75em;padding:7px 8px;outline:none}
input[type=text]:focus{border-color:#00ffcc}
.val{font-size:.7em;color:#00ffcc88;min-width:28px;text-align:right}
.statgrid{display:grid;grid-template-columns:1fr 1fr;gap:6px}
.stat{background:#0a0a0a;border:1px solid #1c1c1c;border-radius:8px;padding:8px 10px}
.stat .lbl{font-size:.6em;color:#444;letter-spacing:2px}
.stat .val2{font-size:1em;color:#00ffcc;margin-top:2px}
.bubble{background:#00ffcc11;border:1px solid #00ffcc33;border-radius:8px;padding:8px 10px;
  font-size:.75em;color:#00ffcc88;min-height:32px;margin-bottom:8px;letter-spacing:1px}
.colorrow{display:flex;flex-wrap:wrap;gap:5px;margin-top:6px}
.cc{width:28px;height:28px;border-radius:6px;border:1px solid #222;cursor:pointer;transition:transform .15s}
.cc:hover{transform:scale(1.2)}
</style></head><body>
<h1>&#x25C6; BUDDY ROBOT &#x25C6;</h1>
<div class='sub' id='nameDisplay'>ALIVE DEMO &nbsp;|&nbsp; WIFI CONTROL</div>

<div class='card'>
  <h2>&#x1F4AC; SPEECH BUBBLE</h2>
  <div class='bubble' id='lastMsg'>No message yet...</div>
  <div class='row'>
    <input type='text' id='msgIn' placeholder='Type anything...' maxlength='18'>
    <button style='width:70px' onclick='sendMsg()'>SEND</button>
  </div>
  <div class='grid' style='margin-top:6px'>
    <button onclick='phrase(0)'>Hello!</button>
    <button onclick='phrase(1)'>I am BUDDY</button>
    <button onclick='phrase(2)'>Beep boop!</button>
    <button onclick='phrase(3)'>How are you?</button>
    <button onclick='phrase(4)'>I feel great!</button>
    <button onclick='phrase(5)'>Whoa!</button>
    <button onclick='phrase(6)'>Zzz...</button>
    <button onclick='phrase(7)'>I like you!</button>
    <button onclick='phrase(8)'>Error 404</button>
    <button onclick='phrase(9)'>Lets party!</button>
    <button onclick='phrase(10)'>Ouch!</button>
    <button onclick='phrase(11)'>Hehe ;)</button>
  </div>
</div>

<div class='card'>
  <h2>&#x25B6; EXPRESSION</h2>
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
    <button onclick='cmd("blink")'>&#x1F441; BLINK</button>
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
</div>

<div class='card'>
  <h2>&#x1F441; EYE CUSTOMIZATION</h2>
  <div class='row'><label>EYE COLOR</label>
    <input type='color' id='ecol' value='#00ffff' oninput='sendEyeColor(this.value)'>
  </div>
  <div class='row'><label>WIDTH</label>
    <input type='range' min='10' max='45' value='28' id='eW' oninput='eyeParam("eyeW",this.value,this)'>
    <span class='val' id='eWv'>28</span>
  </div>
  <div class='row'><label>HEIGHT</label>
    <input type='range' min='8' max='55' value='38' id='eH' oninput='eyeParam("eyeH",this.value,this)'>
    <span class='val' id='eHv'>38</span>
  </div>
  <div class='row'><label>ROUNDNESS</label>
    <input type='range' min='0' max='20' value='7' id='eR' oninput='eyeParam("eyeR",this.value,this)'>
    <span class='val' id='eRv'>7</span>
  </div>
  <div class='row'><label>GAP</label>
    <input type='range' min='2' max='30' value='10' id='eG' oninput='eyeParam("eyeG",this.value,this)'>
    <span class='val' id='eGv'>10</span>
  </div>
</div>

<div class='card'>
  <h2>&#x1F4A1; RGB LIGHT</h2>
  <div class='row'><label>COLOR</label>
    <input type='color' id='cpick' value='#00b4b4' oninput='sendColor(this.value)'>
  </div>
  <div class='row'><label>BRIGHTNESS</label>
    <input type='range' min='0' max='100' value='100' id='bri' oninput='cmd("bri="+this.value);document.getElementById("briv").innerText=this.value'>
    <span class='val' id='briv'>100</span>
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
    <div class='cc' style='background:#111;border-color:#444' onclick='setRGBQ(0,0,0)'>&#x2715;</div>
  </div>
</div>

<div class='card'>
  <h2>&#x1F50A; BUZZER</h2>
  <div class='row'><label>FREQUENCY</label>
    <input type='range' min='500' max='5000' value='3500' id='bfreq' oninput='document.getElementById("bfreqv").innerText=this.value'>
    <span class='val' id='bfreqv'>3500</span>
  </div>
  <div class='row'><label>DURATION</label>
    <input type='range' min='20' max='800' value='200' id='bdur' oninput='document.getElementById("bdurv").innerText=this.value+"ms"'>
    <span class='val' id='bdurv'>200ms</span>
  </div>
  <button style='margin-bottom:8px' onclick='cmd("beep="+document.getElementById("bfreq").value+"&bdur="+document.getElementById("bdur").value)'>&#x25B6; PLAY TONE</button>
  <div class='grid'>
    <button onclick='cmd("melody=0")'>&#x1F3B5; JINGLE</button>
    <button onclick='cmd("melody=1")'>&#x1F3AE; MARIO</button>
    <button onclick='cmd("melody=2")'>&#x1F614; SAD TUNE</button>
    <button onclick='cmd("melody=3")'>&#x26A0; ALERT</button>
    <button onclick='cmd("melody=4")'>&#x1F600; HAPPY</button>
    <button onclick='cmd("melody=5")'>&#x1F916; R2D2</button>
    <button onclick='cmd("buzz_toggle")' id='btog'>&#x1F507; MUTE</button>
  </div>
</div>

<div class='card'>
  <h2>&#x2699; ROBOT NAME</h2>
  <div class='row'>
    <input type='text' id='nameIn' placeholder='Enter name (max 23)' maxlength='23'>
    <button style='width:70px' onclick='sendName()'>SET</button>
  </div>
</div>

<div class='card'>
  <h2>&#x1F4CA; STATS</h2>
  <div class='statgrid'>
    <div class='stat'><div class='lbl'>UPTIME</div><div class='val2' id='uptime'>--</div></div>
    <div class='stat'><div class='lbl'>FREE HEAP</div><div class='val2' id='heap'>--</div></div>
    <div class='stat'><div class='lbl'>CLIENTS</div><div class='val2' id='clients'>--</div></div>
    <div class='stat'><div class='lbl'>EXPRESSIONS</div><div class='val2' id='exprct'>--</div></div>
    <div class='stat'><div class='lbl'>MESSAGES</div><div class='val2' id='msgct'>--</div></div>
    <div class='stat'><div class='lbl'>BUZZER</div><div class='val2' id='buzzst'>--</div></div>
  </div>
</div>

<script>
function cmd(c){fetch('/cmd?'+c);}
function expr(e){cmd('expr='+e);}
function phrase(i){cmd('phrase='+i);document.getElementById('lastMsg').innerText=phrases[i];}
function setMode(m){
  cmd('mode='+m);
  document.querySelectorAll('.pm').forEach((b,i)=>b.classList.toggle('act',i==m));
}
function sendMsg(){
  let t=document.getElementById('msgIn').value.trim();
  if(!t)return;
  cmd('msg='+encodeURIComponent(t));
  document.getElementById('lastMsg').innerText=t;
  document.getElementById('msgIn').value='';
}
function sendColor(hex){
  let r=parseInt(hex.slice(1,3),16),g=parseInt(hex.slice(3,5),16),b=parseInt(hex.slice(5,7),16);
  cmd('rgb='+r+','+g+','+b);
  document.getElementById('rr').value=r;document.getElementById('rrv').innerText=r;
  document.getElementById('rg').value=g;document.getElementById('rgv').innerText=g;
  document.getElementById('rb').value=b;document.getElementById('rbv').innerText=b;
}
function sendRGB(){
  let r=document.getElementById('rr').value,g=document.getElementById('rg').value,b=document.getElementById('rb').value;
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
  let r=parseInt(hex.slice(1,3),16),g=parseInt(hex.slice(3,5),16),b=parseInt(hex.slice(5,7),16);
  cmd('eyecol='+r+','+g+','+b);
}
function eyeParam(p,v,el){
  cmd(p+'='+v);
  document.getElementById(el.id+'v').innerText=v;
}
function sendName(){
  let n=document.getElementById('nameIn').value.trim();
  if(!n)return;
  cmd('name='+encodeURIComponent(n));
  document.getElementById('nameDisplay').innerText=n+' | WIFI CONTROL';
}
const phrases=['Hello!','I am BUDDY','Beep boop!','How are you?','I feel great!','Whoa!','Zzz...','I like you!','Error 404','Lets party!','Ouch!','Hehe ;)'];
setInterval(()=>{
  fetch('/status').then(r=>r.json()).then(d=>{
    document.getElementById('uptime').innerText=d.uptime;
    document.getElementById('heap').innerText=d.heap+'KB';
    document.getElementById('clients').innerText=d.clients;
    document.getElementById('exprct').innerText=d.expressions;
    document.getElementById('msgct').innerText=d.messages;
    document.getElementById('buzzst').innerText=d.buzzer?'ON':'MUTED';
  }).catch(()=>{});
},2500);
</script>
</body></html>
)rawhtml";
