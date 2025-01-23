# thirst --- 口渴值插件

### 基于 C++ 编写提供更高效率的体验

### 基于命令和 molang 提供高度自定义的配置文件

#### 示例配置文件

```jsonc
{
  "version": 0,
  "modifications": {
    "apple": { "value": 10, "commands": ["say eat apple"] }, //吃苹果时触发
    "water": { "value": 1, "commands": ["say drink water"] } //使用物品点击水面触发
  },
  "commands": {
    //键为molang
    "v.r=(v.current_thirst==10);v.r?(v.current_thirst=20);return v.r;": [
      "kill @s" //可运行命令
    ],
    "(v.current_thirst=v.current_thirst-1);return 0;": [], //修改v.current_thirst可直接改变口渴值
    "v.r=(query.is_biome('desert'));v.r?(v.current_thirst=v.current_thirst-1);return v.r;": [
      "say in desert"
    ] //添加新molang可检测玩家所处生物群系
  },
  "thirst_base": 20, //基础口渴值
  "thirst_max": 20, //最大口渴值
  "thirst_min": 0, //最小口渴值
  "thirst_tick": 20, //更新间隔
  "texts": {
    //口渴值不同时显示的文本
    //可使用类似https://www.minebbs.com/resources/moisture.2734/的资源包
    "10": "",
    "11": "",
    "12": "",
    "13": "",
    "14": "",
    "15": "",
    "16": "",
    "17": "",
    "18": "",
    "19": "",
    "20": ""
  }
}
```

### remote call 支持，可与 LSE 脚本插件交互

#### 脚本定义

```javascript
const thirst_api = {
  /**
   *  @type {function(string):number}
   **/
  getThirst: ll.import("thirst", "getThirst"),
  /**
   *  @type {function(string,number):void}
   **/
  setThirst: ll.import("thirst", "setThirst"),
  /**
   *  @type {function(string,Boolean):void}
   **/
  setShowThirst: ll.import("thirst", "setShowThirst")
};
/**
 * @param {string} uuid
 * @returns {number}
 */
function getThirst(uuid) {
  return thirst_api.getThirst(uuid);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {number}
 */
function setThirst(uuid, value) {
  return thirst_api.setThirst(uuid, value);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {number}
 */
function addThirst(uuid, value) {
  return setThirst(uuid, getThirst(uuid) + value);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {number}
 */
function reduceThirst(uuid, value) {
  return setThirst(uuid, getThirst(uuid) - value);
}
/**
 * @param {string} uuid
 * @param {Boolean} value
 * @returns {number}
 */
function setShowThirst(uuid, value) {
  return thirst_api.setShowThirst(uuid, value);
}
```
