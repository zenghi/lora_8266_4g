const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\"content=\"width=device-width,initial-scale=1,user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style> .c{text-align:center;}div,input{padding:5px;font-size:1em;}input{width:95%;}body{text-align:center;font-family:verdana;}button{border:0;border-radius:0.3rem;background-color:#30b477;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float:right;width:64px;text-align:right;} .l{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\")no-repeat left center;background-size:1em;} </style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();} </script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style=\"text-align:left;display:inline-block;min-width:260px;\">";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\"method=\"get\"><button>自动扫描配置WIFI</button></form><br/><form action=\"/0wifi\"method=\"get\"><button>手动输入WIFI名称密码</button></form><br/><form action=\"/i\"method=\"get\"><button>配置信息</button></form><br/><form action=\"/r\"method=\"post\"><button>重启</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p'onclick='c(this)'>{v}</a>&nbsp;<span class='q{i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get'action='wifisave'><input id='s'name='s'length=32 placeholder='名称'><br/><input id='p'name='p'length=64 type='password'placeholder='密码'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}'name='{n}'length={l}placeholder='{p}'value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>保存</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>Credentials Saved<br/>正在连接网络<br/>如果指示灯长时间不停止闪烁，请确认账户密码正常性，重新进行配置WIFI</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";
const char HTTP_FORM_SERVER[] PROGMEM      = "<form method='get'action='serversave'><input id='ip'name='ip'length=32 placeholder='服务器地址'><br/><input id='po'name='po'length=64 type='port'placeholder='服务器端口'><br/>";
const char HTTP_FORM_SAVE[] PROGMEM        = "<br/><button type='submit1'>保存</button></form>";

