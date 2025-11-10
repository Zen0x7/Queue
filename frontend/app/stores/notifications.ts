import {defineStore, acceptHMRUpdate} from "pinia";
import { CheckCircleIcon, XCircleIcon } from '@heroicons/vue/24/outline'
import { v4 as uuid } from 'uuid'

export interface StoredNotification {
    id: string;
    title: string;
    body: string;
    icon: Component;
    color: string;
    timeout: number;
    visible: boolean;
}

export interface Notification {
    title: string;
    body: string;
    icon: Component;
    color: string;
    timeout: number;
}

export interface NotificationRequest {
    title: string;
    body: string;
    timeout: number;
}

export const useNotifications = defineStore("notifications", () => {

    const items : Ref<Array<StoredNotification>> = ref([]);

    const push = async (notification: Notification) =>  {
        const id = uuid();
        items.value.push({
            ...notification,
            visible: true,
            id: id,
        });

        setTimeout(() => {
            const index = items.value.findIndex(item => item.id === id);
            if (index !== -1) {
                items.value[index].visible = false;
            }
        }, notification.timeout)
    }

    const success = async (notification: NotificationRequest) => {
        await push({
            ...notification,
            icon: CheckCircleIcon,
            color: 'text-green-400',
        })
    }

    const error = async (notification: NotificationRequest) => {
        await push({
            ...notification,
            icon: XCircleIcon,
            color: 'text-rose-400',
        });
    }

    return {
        items,
        push,
        success,
        error,
    };
})

if (import.meta.hot) {
    import.meta.hot.accept(acceptHMRUpdate(useNotifications, import.meta.hot))
}