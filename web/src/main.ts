import "cesium/Build/Cesium/Widgets/widgets.css";
import "./style.css";

import {
  Viewer,
  Ion,
  createWorldTerrainAsync,
  ScreenSpaceEventHandler,
  ScreenSpaceEventType,
  defined,
  JulianDate,
  ClockStep,
  CameraEventType,
  KeyboardEventModifier,
} from "cesium";

import { connectBridge } from "./qt-bridge.js";
import { CzmlHandler } from "./czml-handler.js";
import { LiveTracker } from "./live-tracker.js";

async function init(): Promise<void> {
  // Connect to Qt bridge first to get the access token before creating the Viewer
  let bridge: Awaited<ReturnType<typeof connectBridge>> | null = null;
  try {
    bridge = await connectBridge();
    if (bridge.cesiumToken) {
      Ion.defaultAccessToken = bridge.cesiumToken;
    }
  } catch (err) {
    console.warn("Running without Qt bridge:", err);
  }

  const viewer = new Viewer("cesiumContainer", {
    terrainProvider: await createWorldTerrainAsync(),
    animation: true,
    timeline: true,
    baseLayerPicker: false,
    geocoder: false,
    homeButton: false,
    sceneModePicker: false,
    navigationHelpButton: false,
    fullscreenButton: false,
    selectionIndicator: true,
    infoBox: true,
  });

  // Swap middle/right mouse: middle = zoom, right = tilt/pan
  const cameraController = viewer.scene.screenSpaceCameraController;
  cameraController.zoomEventTypes = [
    CameraEventType.MIDDLE_DRAG,
    CameraEventType.WHEEL,
    CameraEventType.PINCH,
  ];
  cameraController.tiltEventTypes = [
    CameraEventType.RIGHT_DRAG,
    CameraEventType.PINCH,
    { eventType: CameraEventType.LEFT_DRAG, modifier: KeyboardEventModifier.CTRL },
    { eventType: CameraEventType.RIGHT_DRAG, modifier: KeyboardEventModifier.CTRL },
  ];

  const liveTracker = new LiveTracker(viewer, 30);
  const czmlHandler = new CzmlHandler(viewer);
  czmlHandler.setLiveTracker(liveTracker);

  // Click handler for marker selection
  const handler = new ScreenSpaceEventHandler(viewer.scene.canvas);
  handler.setInputAction(
    (movement: ScreenSpaceEventHandler.PositionedEvent) => {
      const picked = viewer.scene.pick(movement.position);
      if (defined(picked) && defined(picked.id)) {
        viewer.selectedEntity = picked.id;
      }
    },
    ScreenSpaceEventType.LEFT_CLICK,
  );

  // Wire bridge signals if connected
  if (bridge) {
    bridge.cesiumTokenChanged.connect(() => {
      Ion.defaultAccessToken = bridge!.cesiumToken;
    });

    bridge.czmlReady.connect((czmlJson: string, requestId: number) => {
      if (requestId !== bridge!.pendingRequestId) return;
      czmlHandler.handleBulkLoad(czmlJson).catch(console.error);
    });

    bridge.czmlPacket.connect((czmlJson: string) => {
      czmlHandler.handleLivePacket(czmlJson).catch(console.error);
    });

    bridge.liveModeChanged.connect((liveMode: boolean) => {
      if (liveMode) {
        // Sync clock to real-time so PathGraphics trail aligns with live samples
        viewer.clock.currentTime = JulianDate.now();
        viewer.clock.shouldAnimate = true;
        viewer.clock.multiplier = 1;
        viewer.clock.clockStep = ClockStep.SYSTEM_CLOCK;
        liveTracker.start();
      } else {
        liveTracker.stop();
      }
    });

    // Start live tracker if already in live mode at init
    if (bridge.liveMode) {
      viewer.clock.currentTime = JulianDate.now();
      viewer.clock.shouldAnimate = true;
      viewer.clock.multiplier = 1;
      viewer.clock.clockStep = ClockStep.SYSTEM_CLOCK;
      liveTracker.start();
    }

    console.log("CesiumBridge connected successfully");
  }
}

init().catch(console.error);
