import type { CesiumBridgeProxy } from "./types.js";

// QWebChannel is loaded as a global via <script> tag in index.html
declare const QWebChannel: new (
  transport: unknown,
  callback: (channel: { objects: Record<string, CesiumBridgeProxy> }) => void,
) => void;

export function connectBridge(): Promise<CesiumBridgeProxy> {
  return new Promise((resolve, reject) => {
    if (typeof qt === "undefined" || !qt.webChannelTransport) {
      reject(
        new Error(
          "qt.webChannelTransport not available — not running inside QtWebEngine",
        ),
      );
      return;
    }

    new QWebChannel(qt.webChannelTransport, (channel) => {
      const bridge = channel.objects["CesiumBridge"];
      if (!bridge) {
        reject(new Error("CesiumBridge not found in WebChannel objects"));
        return;
      }
      resolve(bridge);
    });
  });
}
