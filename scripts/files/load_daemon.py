import threading as thr
import time
import os
import tomllib
import uuid
import wiitools as w # type: ignore

# 1 reload

tasks = []

def daemon(app):
    print("daemon")
    while True:
        if not tasks == []:
            match tasks[0]:


                case 1: # reload
                    print("reload")
                    app.apps = []
                    try:
                        folders = os.listdir("apps")
                        print(folders)
                    except FileNotFoundError:
                        print("no")
                        folders = []

                    for i in range(len(folders)):
                        folders[i] = os.path.join("apps", folders[i])

                    for folder in folders:
                        if not os.path.exists(os.path.join(folder, "main.py")):
                            print("continue")
                            continue
                        pre_app = app.app_info(None, None, None,
                                               folder, os.path.join(folder, "main.py"),
                                               None, None)

                        if os.path.exists(os.path.join(folder, "info.toml")):
                            with open(os.path.join(folder, "info.toml"), "rb") as f:
                                toml = tomllib.load(f)

                            info_name = ["name", "owner", "date", "description"]
                            info_data = []

                            for inf in info_name:
                                try:
                                    info_data.append(toml[inf])
                                except KeyError:
                                    info_data.append(None)

                            pre_app.name = info_data[0]
                            pre_app.owner= info_data[1]
                            pre_app.date = info_data[2]
                            pre_app.description = info_data[3]

                        if os.path.exists(os.path.join(folder, "icon.png")):
                            try:
                                png = str(uuid.uuid1())
                                w.png_load_named(os.path.join(folder, "icon.png"), png)
                                pre_app.icon = png
                            except:
                                pass

                        app.apps.append(pre_app)
                    print("finish")
                    app.reloaded = True
                    del tasks[0]
        time.sleep(0.1)

        

                        




                        

                        





def reload():
    print("123")
    tasks.append(1)


def init(app):
    th = thr.Thread(target=daemon, args=(app,))

    th.start()