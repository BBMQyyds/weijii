'use strict';

//开始对弈按钮
function start() {

    if (state === -1) {
        document.getElementById('start').innerHTML = '开始匹配';
        state = 0;
        //清除定时器
        clearInterval(matchid);
        return;
    }

    if (state === 1) {
        alert('对局正在进行中');
        return;
    }

    //开始匹配
    if (state === 0) {
        state = -1;
        document.getElementById('start').innerHTML = '匹配中...';

        let model = $("input[name='model']:checked").val();
        console.log(model);

        // 使用 setInterval 替代 while 循环
        matchid = setInterval(function () {
            $.ajax({
                type: "GET",
                url: "http://101.43.255.130:8080/wait?token=" + localStorage.getItem("token") + "&type=" + model,
                success: function (data) {
                    console.log(data);
                    //去除换行符
                    data = data.replace(/[\r\n]/g, "");
                    if (data.indexOf("waiting...") === -1) {
                        console.log("data: " + data);
                        console.log(data.indexOf("waiting..."));
                        clearInterval(matchid);

                        state = 0;
                        if (data.indexOf("Wait success:") !== -1) {
                            document.getElementById("textarea2").innerHTML = "黑棋";
                        } else {
                            document.getElementById("textarea2").innerHTML = "白棋";
                        }
                        document.getElementById("textarea3").innerHTML = data.split(": ")[1];
                        document.getElementById('start').innerHTML = '对弈中...';

                        if (state === 0) {
                            initqipan();
                            if (state === 0) {
                                state = 1;
                                model = $("input[name='model']:checked").val();
                                valTOstr(model);
                                startmusic.play();
                            } else {
                                model = $("input[name='model']:checked").val();
                                valTOstr(model);
                                startmusic.play();
                            }

                            //设置黑暗森林效果
                            if (model !== 'dark') {
                                document.getElementById('qipan').style.opacity = '1';
                                document.getElementById('miniqipan').style.opacity = '1';
                            } else {
                                document.getElementById('qipan').style.opacity = '0.5';
                                document.getElementById('miniqipan').style.opacity = '0.5';
                            }

                            document.getElementById('pattern').style.display = 'none';
                            document.getElementById('pattern2').style.display = 'block';
                        } else {
                            alert('对局正在进行中');
                        }
                        luoziId = setInterval(function () {
                            $.ajax({
                                type: "GET",
                                url: "http://101.43.255.130:8080/luozi?token=" + localStorage.getItem("token") + "&type=" + model + "&other=wait",
                                success: function (data) {
                                    console.log(data);
                                    data = data.replace(/[\r\n]/g, "");
                                    if (data.indexOf("win the game") === -1) {
                                        if (data === luoziWait) {
                                            console.log("等待对手落子");
                                        } else {
                                            console.log("对手落子");
                                            luoziWait = data;
                                            let ox = data.split(":")[1];
                                            let oy = data.split(":")[2];
                                            if (ox === null || oy === null || ox === undefined || oy === undefined) {
                                                return;
                                            }
                                            let e = {
                                                offsetX: ox,
                                                offsetY: oy,
                                                diy: true
                                            };
                                            console.log(e);
                                            normalluozi(e);
                                        }
                                    } else {
                                        alert("您的对手已认输");
                                        clearInterval(luoziId);
                                        initqipan();
                                        if (state === 1) {
                                            state = 0;
                                        }
                                        document.getElementById('pattern').style.display = 'block';
                                        document.getElementById('pattern2').style.display = 'none';
                                        document.getElementById('start').innerHTML = '开始匹配';
                                    }
                                },
                                // error: function (data) {
                                //     alert("您的对手已认输");
                                //     clearInterval(luoziId);
                                //     initqipan();
                                //     if (state === 1) {
                                //         state = 0;
                                //     }
                                //     document.getElementById('pattern').style.display = 'block';
                                //     document.getElementById('pattern2').style.display = 'none';
                                //     document.getElementById('start').innerHTML = '开始匹配';
                                // }
                            });
                        }, 1000);
                    }
                }
            });
        }, 1000);
    }
}


function fail() {
    model = $("input[name='model']:checked").val();
    valTOstr(model);

    pattern = $("input[name='format']:checked").val();

    let data, name, type;

    if (pattern === 'json') {
        data = JSON.stringify(qipu);
        name = 'json棋谱';
        if (model !== 'move') {
            type = 'json';
        } else {
            type = 'mjson';
        }

    } else if (pattern === 'sgf') {
        data = toSGF(qipu);
        name = 'sgf棋谱';
        if (model !== 'move') {
            type = 'sgf';
        } else {
            type = 'msgf';
        }
    }
    $.ajax({
        type: "GET",
        url: "http://101.43.255.130:8080/lose?token=" + localStorage.getItem("token") + "&type=" + model + "&other=" + data,
        success: function (data) {
            console.log(data);
            data = data.replace(/[\r\n]/g, "");
            if (data.indexOf("lose success") !== -1) {
                alert("认输成功");
                if (state === 1) {
                    initqipan();
                    if (state === 1) {
                        state = 0;
                    }
                    document.getElementById('pattern').style.display = 'block';
                    document.getElementById('pattern2').style.display = 'none';
                    document.getElementById('start').innerHTML = '开始匹配';
                } else {
                    alert('对局尚未开始');
                }
            } else {
                alert("认输失败");
            }
        }
    });
}

//音乐按钮
function musicPlay() {
    let backmusic = document.getElementById('backmusic');
    if (backmusic.paused) {
        backmusic.play();
    } else {
        backmusic.pause();
    }
}

function restoreClick() {
    if (model === 'noji' || model === 'dark') {
        alert('无鸡之地和黑暗鸡林模式下禁用小棋盘');
        return;
    }
    restore();
}

//复原按钮
function restore() {

    if (model === 'move') {
        alert('该模式下暂不支持该功能');
        return;
    }

    for (const qizi of miniqizis) {
        qizi.remove();
    }

    minicount = 0;
    miniMap.clear();
    antiminiMap.clear();
    miniqipu = [];
    window.minijiebu = null;

    for (let i = 0; i < 19; i++) {
        for (let j = 0; j < 19; j++) {
            minichess[i][j] = 0;
        }
    }

    if (model === 'move') {
        //记录拿起的棋子
        upminiqi = null;
        //判断是否移动棋子中
        minimoving = false;
        //鸡动模式下记录当前阶段
        minimovePeriod = false;
    }

    if (model !== 'move') {
        restoreNotMove();
    } else {
        restoreIsMove();
    }


    same = true;
}

//悔棋按钮
function regret() {

    if (model === 'move') {
        alert('该模式下暂不支持该功能');
        return;
    }

    if (model !== 'move') {
        regretNotMove();
    } else {
        regretIsMove();
    }

    back();
}

function backClick() {
    if (model === 'noji' || model === 'dark') {
        alert('无鸡之地和黑暗鸡林模式下禁用小棋盘');
        return;
    }
    back();
}

//回退1步
function back() {

    if (model === 'move') {
        alert('该模式下暂不支持该功能');
        return;
    }

    if (model !== 'move') {
        backNotMove();
    } else {
        backIsMove();
    }
}

function backkClick() {
    if (model === 'noji' || model === 'dark') {
        alert('无鸡之地和黑暗鸡林模式下禁用小棋盘');
        return;
    }
    backk();
}

//回退5步
function backk() {

    if (model === 'move') {
        alert('该模式下暂不支持该功能');
        return;
    }

    for (let i = 0; i < 5; i++) {
        back();
    }
}

function forwardClick() {
    if (model === 'noji' || model === 'dark') {
        alert('无鸡之地和黑暗鸡林模式下禁用小棋盘');
        return;
    }
    forward();
}

//前进1步
function forward() {

    if (model === 'move') {
        alert('该模式下暂不支持该功能');
        return;
    }

    if (model !== 'move') {
        forwardNotMove();
    } else {
        forwardIsMove();
    }
}

function forwarddClick() {
    if (model === 'noji' || model === 'dark') {
        alert('无鸡之地和黑暗鸡林模式下禁用小棋盘');
        return;
    }
    forwardd();
}

//前进5步
function forwardd() {

    if (model === 'move') {
        alert('该模式下暂不支持该功能');
        return;
    }

    for (let i = 0; i < 5; i++) {
        forward();
    }
}

//导出棋谱按钮
function exportqipu() {

    model = $("input[name='model']:checked").val();
    valTOstr(model);

    pattern = $("input[name='format']:checked").val();

    let data, name, type;

    if (pattern === 'json') {
        data = JSON.stringify(qipu);
        name = 'json棋谱';
        if (model !== 'move') {
            type = 'json';
        } else {
            type = 'mjson';
        }

    } else if (pattern === 'sgf') {
        data = toSGF(qipu);
        name = 'sgf棋谱';
        if (model !== 'move') {
            type = 'sgf';
        } else {
            type = 'msgf';
        }
    }

    let link = document.createElement("a");
    let exportName = name ? name : 'data';
    link.href = 'data:text/' + type + ';charset=utf-8,\uFEFF' + encodeURI(data);
    link.download = exportName + "." + type;
    link.click();
}

//导入棋谱按钮
function importqipu() {

    model = $("input[name='model']:checked").val();
    valTOstr(model);

    let file = document.getElementById('btn_file');
    file.click();

}

function adjust() {
    let numericInput = document.getElementById('numericInput');
    let adjust = document.getElementById('miniimage');
    adjust.style.marginRight = numericInput.value + 'px';
}
